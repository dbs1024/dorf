// Copyright (c) Darrin Stewart. All rights reserved.
#include "Engine.Render/Rhi/RhiDevice.h"
#include "D3D12Device.h"

#include <cstdio>

RhiError createCommandQueue(RhiCommandQueue& outQueue, ID3D12Device_t* device, D3D12_COMMAND_LIST_TYPE type)
{
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = type;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	HRESULT hr = device->CreateCommandQueue(&desc, IID_PPV_ARGS(&outQueue.queue));
	if (FAILED(hr))
	{
		printf("createCommandQueue: CreateCommandQueue failed (hr=0x%08X)\n", hr);
		return RhiError::Failed;
	}

	hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&outQueue.fence));
	if (FAILED(hr))
	{
		printf("createCommandQueue: CreateFence failed (hr=0x%08X)\n", hr);
		return RhiError::Failed;
	}

	outQueue.fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (!outQueue.fenceEvent)
	{
		printf("createCommandQueue: CreateEvent failed\n");
		return RhiError::Failed;
	}

	if (createFixedItemPool(outQueue.commandListPool, sizeof(RhiCommandList), kRhiCommandListPoolSize) != FixedItemPoolResult::Success)
	{
		printf("createCommandQueue: createFixedItemPool failed\n");
		return RhiError::Failed;
	}

	return RhiError::Ok;
}

void destroyCommandQueue(RhiCommandQueue* queue)
{
	destroyFixedItemPool(queue->commandListPool);

	if (queue->fenceEvent)
	{
		CloseHandle(queue->fenceEvent);
		queue->fenceEvent = nullptr;
	}
	if (queue->fence)
	{
		queue->fence->Release();
		queue->fence = nullptr;
	}
	if (queue->queue)
	{
		queue->queue->Release();
		queue->queue = nullptr;
	}
}

void garbageCollectCommandQueue(RhiCommandQueue* queue)
{
	if (!queue->fence)
		return;

	uint64_t completedSubmissionIndex = queue->fence->GetCompletedValue();

	unsigned i = 0;
	while (i < queue->inFlightCommandListCount)
	{
		InFlightCommandList& inFlight = queue->inFlightCommandLists[i];

		bool isReclaimable;
		if (inFlight.submissionIndex == 0)
		{
			// Command lists must be closed within the frame they were opened in — cross-frame deferral is not allowed.
			ACE_ASSERT(!inFlight.commandList->isOpen);
			isReclaimable = !inFlight.commandList->isOpen;
		}
		else
		{
			isReclaimable = (inFlight.submissionIndex <= completedSubmissionIndex);
		}

		if (!isReclaimable)
		{
			++i;
			continue;
		}

		destroyRhiCommandList(inFlight.commandList);
		freeFixedItem(queue->commandListPool, inFlight.commandList->selfHandle);

		queue->inFlightCommandListCount--;
		queue->inFlightCommandLists[i] = queue->inFlightCommandLists[queue->inFlightCommandListCount];
	}
}

void waitForCommandQueueIdle(RhiCommandQueue* queue)
{
	if (queue->fence && queue->fence->GetCompletedValue() < queue->lastSubmissionIndex)
	{
		ResetEvent(queue->fenceEvent);
		queue->fence->SetEventOnCompletion(queue->lastSubmissionIndex, queue->fenceEvent);
		WaitForSingleObject(queue->fenceEvent, INFINITE);
	}

	while (queue->inFlightCommandListCount > 0)
	{
		InFlightCommandList& inFlight = queue->inFlightCommandLists[queue->inFlightCommandListCount - 1];
		ACE_ASSERT(!inFlight.commandList->isOpen);

		destroyRhiCommandList(inFlight.commandList);
		freeFixedItem(queue->commandListPool, inFlight.commandList->selfHandle);

		queue->inFlightCommandListCount--;
	}
}

void destroyRhiCommandList(RhiCommandList* commandList)
{
	if (commandList->commandList)
	{
		commandList->commandList->Release();
		commandList->commandList = nullptr;
	}
	if (commandList->allocator)
	{
		commandList->allocator->Release();
		commandList->allocator = nullptr;
	}
}

RhiCommandList* rhiOpenCommandList(RhiDevice* device, RhiCommandListType type)
{
	D3D12_COMMAND_LIST_TYPE d3dType = (type == RhiCommandListType::Compute) ? D3D12_COMMAND_LIST_TYPE_COMPUTE : D3D12_COMMAND_LIST_TYPE_DIRECT;
	RhiCommandQueue& queue = (type == RhiCommandListType::Compute) ? device->computeQueue : device->graphicsQueue;

	FixedItemHandle handle;
	void* ptr = allocFixedItem(handle, queue.commandListPool);
	if (!ptr)
	{
		ACE_ASSERT(false);
		return nullptr;
	}

	RhiCommandList* commandList = static_cast<RhiCommandList*>(ptr);
	memset(commandList, 0, sizeof(RhiCommandList));
	commandList->device = device;
	commandList->type   = type;
	commandList->selfHandle = handle;

	HRESULT hr = device->d3dDevice->CreateCommandAllocator(d3dType, IID_PPV_ARGS(&commandList->allocator));
	if (FAILED(hr))
	{
		ACE_ASSERT(false);
		printf("rhiOpenCommandList: CreateCommandAllocator failed (hr=0x%08X)\n", hr);
		freeFixedItem(queue.commandListPool, handle);
		return nullptr;
	}

	hr = device->d3dDevice->CreateCommandList(0, d3dType, commandList->allocator, nullptr, IID_PPV_ARGS(&commandList->commandList));
	if (FAILED(hr))
	{
		ACE_ASSERT(false);
		printf("rhiOpenCommandList: CreateCommandList failed (hr=0x%08X)\n", hr);
		commandList->allocator->Release();
		freeFixedItem(queue.commandListPool, handle);
		return nullptr;
	}

	if (type == RhiCommandListType::Graphics)
	{
		ID3D12DescriptorHeap* heaps[] = { device->gpuCbvSrvUavHeap.heap, device->gpuSamplerHeap.heap };
		commandList->commandList->SetDescriptorHeaps(_countof(heaps), heaps);
	}
	else
	{
		// Non-graphics command lists are not supported yet.
		ACE_ASSERT(false);
	}

	commandList->isOpen = true;

	ACE_ASSERT(queue.inFlightCommandListCount < kRhiInFlightCommandListCount);
	queue.inFlightCommandLists[queue.inFlightCommandListCount] = { commandList, 0 };
	queue.inFlightCommandListCount++;

	return commandList;
}

void rhiCloseCommandList(RhiCommandList* commandList)
{
	ACE_ASSERT(commandList->isOpen);

	ACE_VERIFY_HR(commandList->commandList->Close());
	commandList->isOpen = false;
}

void rhiExecuteCommandList(RhiCommandList* commandList)
{
	ACE_ASSERT(!commandList->isOpen);

	RhiDevice* device = commandList->device;
	RhiCommandQueue& queue = (commandList->type == RhiCommandListType::Compute) ? device->computeQueue : device->graphicsQueue;

	ID3D12CommandList* lists[] = { commandList->commandList };
	queue.queue->ExecuteCommandLists(_countof(lists), lists);

	queue.lastSubmissionIndex++;
	ACE_VERIFY_HR(queue.queue->Signal(queue.fence, queue.lastSubmissionIndex));

	InFlightCommandList* inFlight = nullptr;
	for (unsigned i = 0; i < queue.inFlightCommandListCount; ++i)
	{
		if (queue.inFlightCommandLists[i].commandList == commandList && queue.inFlightCommandLists[i].submissionIndex == 0)
		{
			inFlight = &queue.inFlightCommandLists[i];
			break;
		}
	}
	ACE_ASSERT(inFlight);
	inFlight->submissionIndex = queue.lastSubmissionIndex;
}
