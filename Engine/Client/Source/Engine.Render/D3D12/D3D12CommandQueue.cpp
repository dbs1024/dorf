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

	return RhiError::Ok;
}

void destroyCommandQueue(RhiCommandQueue* queue)
{
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
