// Copyright (c) Darrin Stewart. All rights reserved.
#include "Engine.Render/Rhi/RhiDevice.h"
#include "D3D12Device.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <cstdio>

using Microsoft::WRL::ComPtr;

// Helper precedes caller — no forward declaration.
static ComPtr<IDXGIAdapter1> getHardwareAdapter(IDXGIFactory7* factory)
{
	ComPtr<IDXGIAdapter1> intelFallback;

	ComPtr<IDXGIAdapter1> adapter;
	for (UINT i = 0; factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue;

		if (FAILED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1, _uuidof(ID3D12Device), nullptr)))
			continue;

		if (desc.VendorId == 0x8086) // Intel
		{
			if (!intelFallback)
				intelFallback = adapter;
			continue;
		}

		return adapter;
	}

	return intelFallback;
}

// Helper precedes caller — no forward declaration.
static ComPtr<IDXGIFactory7> initD3dDevice(RhiDevice* device, const RhiDeviceCreateParams& params)
{
	if (params.enableDebug)
	{
		ComPtr<ID3D12Debug> debug;
		HRESULT hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debug));
		if (FAILED(hr))
		{
			printf("RhiDevice: D3D12GetDebugInterface failed (hr=0x%08X)\n", hr);
			return ComPtr<IDXGIFactory7>();
		}
		debug->EnableDebugLayer();
	}

	if (params.enableGpuValidation)
	{
		ComPtr<ID3D12Debug1> debug1;
		HRESULT hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debug1));
		if (FAILED(hr))
		{
			printf("RhiDevice: D3D12GetDebugInterface (ID3D12Debug1) failed (hr=0x%08X)\n", hr);
			return ComPtr<IDXGIFactory7>();
		}
		debug1->SetEnableGPUBasedValidation(true);
	}

	UINT flags = params.enableDebug ? DXGI_CREATE_FACTORY_DEBUG : 0;
	ComPtr<IDXGIFactory7> factory;
	HRESULT hr = CreateDXGIFactory2(flags, IID_PPV_ARGS(&factory));
	if (FAILED(hr))
	{
		printf("RhiDevice: CreateDXGIFactory2 failed (hr=0x%08X)\n", hr);
		return ComPtr<IDXGIFactory7>();
	}

	ComPtr<IDXGIAdapter1> adapter = getHardwareAdapter(factory.Get());
	if (!adapter)
	{
		printf("RhiDevice: no suitable hardware adapter found\n");
		return ComPtr<IDXGIFactory7>();
	}

	hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device->d3dDevice));
	if (FAILED(hr))
	{
		printf("RhiDevice: D3D12CreateDevice failed (hr=0x%08X)\n", hr);
		return ComPtr<IDXGIFactory7>();
	}

	return factory;
}

// Helper precedes caller — no forward declaration.
static RhiError initCommandQueues(RhiDevice* device)
{
	if (createCommandQueue(device->graphicsQueue, device->d3dDevice, D3D12_COMMAND_LIST_TYPE_DIRECT) != RhiError::Ok)
		return RhiError::Failed;

	if (createCommandQueue(device->computeQueue, device->d3dDevice, D3D12_COMMAND_LIST_TYPE_COMPUTE) != RhiError::Ok)
		return RhiError::Failed;

	return RhiError::Ok;
}

// Helper precedes caller — no forward declaration.
static RhiError initDescriptorHeaps(RhiDevice* device)
{
	ID3D12Device_t* d3d = device->d3dDevice;

	if (createDescriptorHeap(device->cpuCbvSrvUavHeap, d3d, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1024, false) != RhiError::Ok)
		return RhiError::Failed;

	if (createDescriptorHeap(device->cpuSamplerHeap, d3d, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 1024, false) != RhiError::Ok)
		return RhiError::Failed;

	if (createDescriptorHeap(device->cpuRtvHeap, d3d, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1024, false) != RhiError::Ok)
		return RhiError::Failed;

	if (createDescriptorHeap(device->cpuDsvHeap, d3d, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1024, false) != RhiError::Ok)
		return RhiError::Failed;

	if (createDescriptorHeap(device->gpuCbvSrvUavHeap, d3d, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1024, true) != RhiError::Ok)
		return RhiError::Failed;

	if (createDescriptorHeap(device->gpuSamplerHeap, d3d, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 1024, true) != RhiError::Ok)
		return RhiError::Failed;

	return RhiError::Ok;
}

// Helper precedes caller — no forward declaration.
static RhiError initFences(RhiDevice* device, const RhiDeviceCreateParams& params)
{
	HRESULT hr = device->d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&device->frameFence));
	if (FAILED(hr))
	{
		printf("initFences: CreateFence failed (hr=0x%08X)\n", hr);
		return RhiError::Failed;
	}

	for (unsigned i = 0; i < params.maxRenderedFrames; ++i)
	{
		device->frameFenceEvents[i] = CreateEvent(nullptr, FALSE, TRUE, nullptr);
		if (!device->frameFenceEvents[i])
		{
			printf("initFences: CreateEvent(%u) failed\n", i);
			return RhiError::Failed;
		}
	}

	return RhiError::Ok;
}

// Helper precedes caller — no forward declaration.
static RhiError initSwapChain(RhiDevice* device, IDXGIFactory7* factory, const RhiDeviceCreateParams& params)
{
	DXGI_SWAP_CHAIN_DESC1 desc = {};
	desc.BufferCount        = kRhiSwapChainImageCount;
	desc.Width              = static_cast<UINT>(params.backbufferWidth);
	desc.Height             = static_cast<UINT>(params.backbufferHeight);
	desc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.SwapEffect         = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	desc.SampleDesc.Count   = 1;

	ComPtr<IDXGISwapChain1> swapChain1;
	HRESULT hr = factory->CreateSwapChainForHwnd(
		device->graphicsQueue.queue,
		static_cast<HWND>(device->window),
		&desc,
		nullptr,
		nullptr,
		&swapChain1);
	if (FAILED(hr))
	{
		printf("initSwapChain: CreateSwapChainForHwnd failed (hr=0x%08X)\n", hr);
		return RhiError::Failed;
	}

	factory->MakeWindowAssociation(static_cast<HWND>(device->window), DXGI_MWA_NO_ALT_ENTER);

	hr = swapChain1->QueryInterface(IID_PPV_ARGS(&device->swapChain));
	if (FAILED(hr))
	{
		printf("initSwapChain: QueryInterface for IDXGISwapChain3 failed (hr=0x%08X)\n", hr);
		return RhiError::Failed;
	}

	device->swapChainImageIndex = device->swapChain->GetCurrentBackBufferIndex();

	for (unsigned i = 0; i < kRhiSwapChainImageCount; ++i)
	{
		void* ptr = allocFixedItem(device->swapChainImages[i], device->resourcePool);
		RhiResource* resource = static_cast<RhiResource*>(ptr);
		hr = device->swapChain->GetBuffer(i, IID_PPV_ARGS(&resource->resource));
		if (FAILED(hr))
		{
			printf("initSwapChain: GetBuffer(%u) failed (hr=0x%08X)\n", i, hr);
			return RhiError::Failed;
		}
		resource->state     = D3D12_RESOURCE_STATE_PRESENT;
		resource->rtvHandle = allocPersistentDescriptor(&device->cpuRtvHeap);

		D3D12_CPU_DESCRIPTOR_HANDLE rtvCpuHandle = getD3D12CpuDescriptorHandle(&device->cpuRtvHeap, resource->rtvHandle);

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format        = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

		device->d3dDevice->CreateRenderTargetView(resource->resource, &rtvDesc, rtvCpuHandle);
	}

	return RhiError::Ok;
}

static void waitForIdle(RhiDevice* device)
{
	waitForCommandQueueIdle(&device->graphicsQueue);
	waitForCommandQueueIdle(&device->computeQueue);

	for (unsigned i = 0; i < kRhiMaxRenderedFrames; ++i)
	{
		if (device->frameFenceEvents[i])
			WaitForSingleObject(device->frameFenceEvents[i], INFINITE);
	}
}

RhiError rhiCreateDevice(RhiDevice** outDevice, const RhiDeviceCreateParams& params)
{
	if (params.maxRenderedFrames > kRhiMaxRenderedFrames)
		return RhiError::InvalidArg;

	RhiDevice* device = new RhiDevice;
	memset(device, 0, sizeof(*device));
	device->window = params.window;
	createFixedItemPool(device->resourcePool, sizeof(RhiResource), 65536);
	for (unsigned i = 0; i < kRhiSwapChainImageCount; ++i)
		device->swapChainImages[i] = InvalidFixedItemHandle;

	ComPtr<IDXGIFactory7> factory = initD3dDevice(device, params);
	if (!factory)
	{
		rhiDestroyDevice(device);
		return RhiError::Failed;
	}

	if (initCommandQueues(device) != RhiError::Ok)
	{
		rhiDestroyDevice(device);
		return RhiError::Failed;
	}

	if (initDescriptorHeaps(device) != RhiError::Ok)
	{
		rhiDestroyDevice(device);
		return RhiError::Failed;
	}

	if (initFences(device, params) != RhiError::Ok)
	{
		rhiDestroyDevice(device);
		return RhiError::Failed;
	}

	if (initSwapChain(device, factory.Get(), params) != RhiError::Ok)
	{
		rhiDestroyDevice(device);
		return RhiError::Failed;
	}

	*outDevice = device;
	return RhiError::Ok;
}

void rhiDestroyDevice(RhiDevice* device)
{
	if (!device)
		return;

	waitForIdle(device);

	for (unsigned i = 0; i < kRhiSwapChainImageCount; ++i)
	{
		if (device->swapChainImages[i] == InvalidFixedItemHandle)
			continue;
		RhiResource* resource = static_cast<RhiResource*>(getFixedItemPtr(device->resourcePool, device->swapChainImages[i]));
		freePersistentDescriptor(&device->cpuRtvHeap, resource->rtvHandle);
		if (resource->resource)
			resource->resource->Release();
		freeFixedItem(device->resourcePool, device->swapChainImages[i]);
	}
	if (device->swapChain)
	{
		device->swapChain->Release();
		device->swapChain = nullptr;
	}
	for (unsigned i = 0; i < kRhiMaxRenderedFrames; ++i)
	{
		if (device->frameFenceEvents[i])
			CloseHandle(device->frameFenceEvents[i]);
	}
	if (device->frameFence)
	{
		device->frameFence->Release();
		device->frameFence = nullptr;
	}
	destroyCommandQueue(&device->graphicsQueue);
	destroyCommandQueue(&device->computeQueue);
	destroyDescriptorHeap(&device->cpuCbvSrvUavHeap);
	destroyDescriptorHeap(&device->cpuSamplerHeap);
	destroyDescriptorHeap(&device->cpuRtvHeap);
	destroyDescriptorHeap(&device->cpuDsvHeap);
	destroyDescriptorHeap(&device->gpuCbvSrvUavHeap);
	destroyDescriptorHeap(&device->gpuSamplerHeap);
	destroyFixedItemPool(device->resourcePool);
	if (device->d3dDevice)
	{
		device->d3dDevice->Release();
		device->d3dDevice = nullptr;
	}
	delete device;
}

void rhiBeginFrame(RhiDevice* device)
{
	ACE_ASSERT(!device->insideFrame);
	device->insideFrame = true;

	unsigned bufferIndex = device->swapChain->GetCurrentBackBufferIndex();
	WaitForSingleObject(device->frameFenceEvents[bufferIndex], INFINITE);
}

void rhiEndFrame(RhiDevice* device)
{
	ACE_ASSERT(device->insideFrame);
	device->insideFrame = false;

	unsigned bufferIndex = device->swapChain->GetCurrentBackBufferIndex();
	device->swapChain->Present(1, 0);

	garbageCollectCommandQueue(&device->graphicsQueue);
	garbageCollectCommandQueue(&device->computeQueue);

	device->frameFence->SetEventOnCompletion(device->frameCount, device->frameFenceEvents[bufferIndex]);

	device->graphicsQueue.queue->Signal(device->frameFence, device->frameCount);
	// TODO: compute queues aren't used yet; revisit how the frame fence should be signaled across multiple queues
	// device->computeQueue.queue->Signal(device->frameFence, device->frameCount);

	device->frameCount++;
}

RhiResource* rhiGetBackBuffer(RhiDevice* device)
{
	unsigned bufferIndex = device->swapChain->GetCurrentBackBufferIndex();
	return static_cast<RhiResource*>(getFixedItemPtr(device->resourcePool, device->swapChainImages[bufferIndex]));
}

RhiResource* allocResource(RhiResourceHandle& outHandle, RhiDevice* device)
{
	void* ptr = allocFixedItem(outHandle, device->resourcePool);
	if (!ptr)
		return nullptr;
	RhiResource* resource = static_cast<RhiResource*>(ptr);
	memset(resource, 0, sizeof(RhiResource));
	return resource;
}

void freeResource(RhiDevice* device, RhiResourceHandle handle)
{
	freeFixedItem(device->resourcePool, handle);
}
