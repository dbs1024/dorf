// Copyright (c) Darrin Stewart. All rights reserved.
#pragma once

#include "Core.Util/FixedItemPool.h"

#include <d3d12.h>
#include <dxgi1_4.h>
#include <cstdint>
#include <type_traits>

enum class RhiError : unsigned;
enum class RhiCommandListType : unsigned;

using ID3D12Device_t = ID3D12Device4;
using RhiDescriptorHandle = int;
using RhiResourceHandle = FixedItemHandle;

// Always evaluates expr (an HRESULT-returning expression); asserts on failure in debug builds and ignores the result in retail builds.
#define ACE_VERIFY_HR(expr) \
	do \
	{ \
		HRESULT aceVerifyHrResult = (expr); \
		ACE_ASSERT(SUCCEEDED(aceVerifyHrResult)); \
		(void)aceVerifyHrResult; \
	} while (false)

constexpr unsigned kRhiMaxRenderedFrames       = 4;
constexpr unsigned kRhiSwapChainImageCount     = 2;
constexpr int      kRhiCommandListPoolSize     = 1024;
constexpr unsigned kRhiInFlightCommandListCount = 1024;

struct RhiResource
{
	ID3D12Resource*     resource;
	D3D12_RESOURCE_STATES state;
	RhiDescriptorHandle rtvHandle;
};

static_assert(std::is_trivially_default_constructible_v<RhiResource> && std::is_trivially_destructible_v<RhiResource>);

struct RhiCommandList
{
	ID3D12GraphicsCommandList* commandList;
	ID3D12CommandAllocator*    allocator;
	RhiDevice*                 device;
	RhiCommandListType         type;
	bool                       isOpen;
	FixedItemHandle            selfHandle;
};

static_assert(std::is_trivially_default_constructible_v<RhiCommandList> && std::is_trivially_destructible_v<RhiCommandList>);

struct InFlightCommandList
{
	RhiCommandList* commandList;
	uint64_t        submissionIndex; // 0 means opened but not yet submitted to rhiExecuteCommandList
};

static_assert(std::is_trivially_default_constructible_v<InFlightCommandList> && std::is_trivially_destructible_v<InFlightCommandList>);

struct RhiCommandQueue
{
	ID3D12CommandQueue* queue;
	ID3D12Fence*        fence;
	HANDLE              fenceEvent;
	FixedItemPoolHandle commandListPool;
	InFlightCommandList inFlightCommandLists[kRhiInFlightCommandListCount];
	unsigned            inFlightCommandListCount;
	uint64_t            lastSubmissionIndex;
};

static_assert(std::is_trivially_default_constructible_v<RhiCommandQueue> && std::is_trivially_destructible_v<RhiCommandQueue>);

struct RhiDescriptorHeap
{
	ID3D12DescriptorHeap*       heap;
	D3D12_DESCRIPTOR_HEAP_TYPE  type;
	unsigned                    descriptorCount;
	unsigned                    persistentDescriptorCount;
	unsigned                    descriptorSize;
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
	RhiDescriptorHandle*        persistentFreeList;
	int                         persistentFreeCount;
};

static_assert(std::is_trivially_default_constructible_v<RhiDescriptorHeap> && std::is_trivially_destructible_v<RhiDescriptorHeap>);

struct RhiRingBufferFenceOffsetData
{
	uint64_t dataOffset;
	uint64_t submittedFrame;
};

struct RhiRingBuffer
{
	uint64_t size;
	uint64_t readOffset;
	uint64_t writeOffset;
	RhiRingBufferFenceOffsetData fenceOffsets[kRhiMaxRenderedFrames];
	unsigned fenceOffsetHead;
	unsigned fenceOffsetCount;
};

static_assert(std::is_trivially_default_constructible_v<RhiRingBuffer> && std::is_trivially_destructible_v<RhiRingBuffer>);

struct RhiDevice
{
	void*               window;
	ID3D12Device_t*     d3dDevice;
	ID3D12Fence*        frameFence;
	IDXGISwapChain3*    swapChain;
	HANDLE              frameFenceEvents[kRhiMaxRenderedFrames];
	unsigned            swapChainImageIndex;
	RhiCommandQueue     graphicsQueue;
	RhiCommandQueue     computeQueue;
	RhiDescriptorHeap   cpuCbvSrvUavHeap;
	RhiDescriptorHeap   cpuSamplerHeap;
	RhiDescriptorHeap   cpuRtvHeap;
	RhiDescriptorHeap   cpuDsvHeap;
	RhiDescriptorHeap   gpuCbvSrvUavHeap;
	RhiDescriptorHeap   gpuSamplerHeap;
	RhiResourceHandle   swapChainImages[kRhiSwapChainImageCount];
	FixedItemPoolHandle resourcePool;
	bool                insideFrame;
	uint64_t            frameCount;
};

static_assert(std::is_trivially_default_constructible_v<RhiDevice> && std::is_trivially_destructible_v<RhiDevice>);

RhiError createCommandQueue(RhiCommandQueue& outQueue, ID3D12Device_t* device, D3D12_COMMAND_LIST_TYPE type);
void     destroyCommandQueue(RhiCommandQueue* queue);
void     garbageCollectCommandQueue(RhiCommandQueue* queue);
void     waitForCommandQueueIdle(RhiCommandQueue* queue);

void     destroyRhiCommandList(RhiCommandList* commandList);

RhiError createDescriptorHeap(RhiDescriptorHeap& outHeap, ID3D12Device_t* device, D3D12_DESCRIPTOR_HEAP_TYPE type, unsigned descriptorCount, bool isGpuHeap);
void     destroyDescriptorHeap(RhiDescriptorHeap* heap);

RhiDescriptorHandle allocPersistentDescriptor(RhiDescriptorHeap* heap);
void                freePersistentDescriptor(RhiDescriptorHeap* heap, RhiDescriptorHandle handle);

D3D12_CPU_DESCRIPTOR_HANDLE getD3D12CpuDescriptorHandle(const RhiDescriptorHeap* heap, RhiDescriptorHandle handle);

RhiResource* allocResource(RhiResourceHandle& outHandle, RhiDevice* device);
void         freeResource(RhiDevice* device, RhiResourceHandle handle);
