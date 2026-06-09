// Copyright (c) Darrin Stewart. All rights reserved.
#include "Engine.Render/Rhi/RhiDevice.h"
#include "D3D12Device.h"

static D3D12_RESOURCE_STATES toD3D12ResourceState(RhiResourceState state)
{
	switch (state)
	{
		case RhiResourceState::Common:                          return D3D12_RESOURCE_STATE_COMMON;
		case RhiResourceState::VertexAndConstantBuffer:         return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		case RhiResourceState::IndexBuffer:                     return D3D12_RESOURCE_STATE_INDEX_BUFFER;
		case RhiResourceState::RenderTarget:                    return D3D12_RESOURCE_STATE_RENDER_TARGET;
		case RhiResourceState::UnorderedAccess:                 return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		case RhiResourceState::DepthWrite:                      return D3D12_RESOURCE_STATE_DEPTH_WRITE;
		case RhiResourceState::DepthRead:                       return D3D12_RESOURCE_STATE_DEPTH_READ;
		case RhiResourceState::NonPixelShaderResource:          return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		case RhiResourceState::PixelShaderResource:             return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		case RhiResourceState::IndirectArgument:                return D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
		case RhiResourceState::CopyDest:                        return D3D12_RESOURCE_STATE_COPY_DEST;
		case RhiResourceState::CopySource:                      return D3D12_RESOURCE_STATE_COPY_SOURCE;
		case RhiResourceState::ResolveDest:                     return D3D12_RESOURCE_STATE_RESOLVE_DEST;
		case RhiResourceState::ResolveSource:                   return D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
		case RhiResourceState::RaytracingAccelerationStructure: return D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
		case RhiResourceState::Present:                         return D3D12_RESOURCE_STATE_PRESENT;
	}
	ACE_ASSERT(false);
	return D3D12_RESOURCE_STATE_COMMON;
}

void rhiTransitionState(RhiCommandList* commandList, RhiResource* resource, RhiResourceState newState)
{
	D3D12_RESOURCE_STATES d3dNewState = toD3D12ResourceState(newState);

	D3D12_RESOURCE_BARRIER barrier        = {};
	barrier.Type                          = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags                         = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource          = resource->resource;
	barrier.Transition.StateBefore        = resource->state;
	barrier.Transition.StateAfter         = d3dNewState;
	barrier.Transition.Subresource        = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	commandList->commandList->ResourceBarrier(1, &barrier);

	resource->state = d3dNewState;
}

void rhiClearRenderTarget(RhiCommandList* commandList, RhiResource* resource, Vector4f color)
{
	ACE_ASSERT(resource->state == D3D12_RESOURCE_STATE_RENDER_TARGET);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = getD3D12CpuDescriptorHandle(&commandList->device->cpuRtvHeap, resource->rtvHandle);

	float clearColor[4] = { color.x, color.y, color.z, color.w };
	commandList->commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
}
