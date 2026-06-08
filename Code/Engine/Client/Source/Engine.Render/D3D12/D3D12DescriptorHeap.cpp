// Copyright (c) Darrin Stewart. All rights reserved.
#include "Engine.Render/Rhi/RhiDevice.h"
#include "D3D12Device.h"

#include <cstdio>

RhiError createDescriptorHeap(RhiDescriptorHeap& outHeap, ID3D12Device_t* device, D3D12_DESCRIPTOR_HEAP_TYPE type, unsigned descriptorCount, bool isGpuHeap)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type           = type;
	desc.NumDescriptors = descriptorCount;
	desc.Flags          = isGpuHeap ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	HRESULT hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&outHeap.heap));
	if (FAILED(hr))
	{
		printf("createDescriptorHeap: CreateDescriptorHeap failed (hr=0x%08X)\n", hr);
		return RhiError::Failed;
	}

	outHeap.type                      = type;
	outHeap.descriptorCount           = descriptorCount;
	outHeap.persistentDescriptorCount = descriptorCount;
	outHeap.descriptorSize            = device->GetDescriptorHandleIncrementSize(type);
	outHeap.cpuHandle                 = outHeap.heap->GetCPUDescriptorHandleForHeapStart();
	outHeap.gpuHandle                 = isGpuHeap ? outHeap.heap->GetGPUDescriptorHandleForHeapStart() : D3D12_GPU_DESCRIPTOR_HANDLE{};

	outHeap.persistentFreeList  = new RhiDescriptorHandle[descriptorCount];
	outHeap.persistentFreeCount = static_cast<int>(descriptorCount);
	for (int i = 0; i < outHeap.persistentFreeCount; ++i)
		outHeap.persistentFreeList[i] = outHeap.persistentFreeCount - 1 - i;

	return RhiError::Ok;
}

void destroyDescriptorHeap(RhiDescriptorHeap* heap)
{
	delete[] heap->persistentFreeList;
	heap->persistentFreeList  = nullptr;
	heap->persistentFreeCount = 0;
	if (heap->heap)
	{
		heap->heap->Release();
		heap->heap = nullptr;
	}
}

RhiDescriptorHandle allocPersistentDescriptor(RhiDescriptorHeap* heap)
{
	ACE_ASSERT(heap->persistentFreeCount > 0);
	return heap->persistentFreeList[--heap->persistentFreeCount] + 1;
}

void freePersistentDescriptor(RhiDescriptorHeap* heap, RhiDescriptorHandle handle)
{
	if (!handle)
		return;
	ACE_ASSERT(static_cast<unsigned>(handle) <= heap->persistentDescriptorCount);
	heap->persistentFreeList[heap->persistentFreeCount++] = handle - 1;
}

D3D12_CPU_DESCRIPTOR_HANDLE getD3D12CpuDescriptorHandle(const RhiDescriptorHeap* heap, RhiDescriptorHandle handle)
{
	ACE_ASSERT(handle > 0 && static_cast<unsigned>(handle) <= heap->persistentDescriptorCount);

	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = heap->cpuHandle;
	cpuHandle.ptr += static_cast<SIZE_T>(handle - 1) * heap->descriptorSize;
	return cpuHandle;
}
