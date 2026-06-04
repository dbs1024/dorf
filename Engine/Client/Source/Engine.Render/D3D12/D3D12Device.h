// Copyright (c) Darrin Stewart. All rights reserved.
#pragma once

#include "Core.Util/FixedItemPool.h"

#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>
#include <cstdint>

enum class RhiError : unsigned;

using RhiDescriptorHandle = int;

constexpr RhiDescriptorHandle InvalidRhiDescriptorHandle = -1;

struct RhiResource
{
	Microsoft::WRL::ComPtr<ID3D12Resource> resource;
	D3D12_RESOURCE_STATES state     = D3D12_RESOURCE_STATE_COMMON;
	RhiDescriptorHandle   rtvHandle = InvalidRhiDescriptorHandle;
};

using RhiResourcePool   = FixedItemPoolT<RhiResource>;
using RhiResourceHandle = FixedItemHandle;

constexpr unsigned kRhiMaxRenderedFrames    = 4;
constexpr unsigned kRhiSwapChainImageCount  = 2;

using ID3D12Device_t = ID3D12Device4;

struct RhiCommandQueue
{
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue;
	Microsoft::WRL::ComPtr<ID3D12Fence> fence;
	HANDLE fenceEvent = nullptr;
};

struct RhiDescriptorHeap
{
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap;
	D3D12_DESCRIPTOR_HEAP_TYPE type        = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	unsigned                   descriptorCount          = 0;
	unsigned                   persistentDescriptorCount = 0;
	unsigned                   descriptorSize           = 0;
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = {};
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = {};
	RhiDescriptorHandle* persistentFreeList  = nullptr;
	int                  persistentFreeCount = 0;
};

struct RhiRingBufferFenceOffsetData
{
	uint64_t dataOffset;
	uint64_t submittedFrame;
};

struct RhiRingBuffer
{
	uint64_t size        = 0;
	uint64_t readOffset  = 0;
	uint64_t writeOffset = 0;
	RhiRingBufferFenceOffsetData fenceOffsets[kRhiMaxRenderedFrames] = {};
	unsigned fenceOffsetHead  = 0;
	unsigned fenceOffsetCount = 0;
};

// Member order is significant: C++ destructs in reverse declaration order, so
// children (swapChain, heaps, queues) are released before the parent d3dDevice.
struct RhiDevice
{
	void* window = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Device_t> d3dDevice;
	RhiCommandQueue   graphicsQueue;
	RhiCommandQueue   computeQueue;
	RhiDescriptorHeap cpuCbvSrvUavHeap;
	RhiDescriptorHeap cpuSamplerHeap;
	RhiDescriptorHeap cpuRtvHeap;
	RhiDescriptorHeap cpuDsvHeap;
	RhiDescriptorHeap gpuCbvSrvUavHeap;
	RhiDescriptorHeap gpuSamplerHeap;
	Microsoft::WRL::ComPtr<ID3D12Fence> frameFence;
	HANDLE                              frameFenceEvents[kRhiMaxRenderedFrames] = {};
	Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain;
	unsigned          swapChainImageIndex                          = 0;
	RhiResourceHandle swapChainImages[kRhiSwapChainImageCount];
	RhiResourcePool   resourcePool;
};

RhiError createCommandQueue(RhiCommandQueue* outQueue, ID3D12Device_t* device, D3D12_COMMAND_LIST_TYPE type);
void     destroyCommandQueue(RhiCommandQueue* queue);

RhiError createDescriptorHeap(RhiDescriptorHeap* outHeap, ID3D12Device_t* device, D3D12_DESCRIPTOR_HEAP_TYPE type, unsigned descriptorCount, bool isGpuHeap);
void     destroyDescriptorHeap(RhiDescriptorHeap* heap);

RhiDescriptorHandle allocPersistentDescriptor(RhiDescriptorHeap* heap);
void                freePersistentDescriptor(RhiDescriptorHeap* heap, RhiDescriptorHandle index);
