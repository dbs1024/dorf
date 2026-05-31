// Copyright (c) Darrin Stewart. All rights reserved.
#include "Engine.Render/Rhi/RhiDevice.h"
#include "D3D12Common.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <cstdio>

using Microsoft::WRL::ComPtr;

struct RhiDevice
{
	void* window;
	ComPtr<ID3D12Device_t> d3dDevice;
};

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

RhiError createRhiDevice(RhiDevice** outDevice, const RhiDeviceCreateParams& params)
{
	RhiDevice* device = new RhiDevice();
	device->window = params.window;

	ComPtr<IDXGIFactory7> factory = initD3dDevice(device, params);
	if (!factory)
	{
		delete device;
		return RhiError::InvalidArg;
	}

	*outDevice = device;
	return RhiError::Ok;
}

void destroyRhiDevice(RhiDevice* device)
{
	if (!device)
		return;
	delete device;
}
