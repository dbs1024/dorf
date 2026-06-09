// Copyright (c) Darrin Stewart. All rights reserved.
#pragma once

#include "Core.Math/Vector4f.h"

struct RhiCommandList;
struct RhiDevice;
struct RhiResource;

struct RhiDeviceCreateParams
{
	void* window;
	bool enableDebug;
	bool enableGpuValidation;
	unsigned maxRenderedFrames;
	int backbufferWidth;
	int backbufferHeight;
};

enum class RhiError : unsigned
{
	Ok,
	InvalidArg,
	Failed,
};

enum class RhiCommandListType : unsigned
{
	Graphics,
	Compute,
};

enum class RhiResourceState : unsigned
{
	Common,
	VertexAndConstantBuffer,
	IndexBuffer,
	RenderTarget,
	UnorderedAccess,
	DepthWrite,
	DepthRead,
	NonPixelShaderResource,
	PixelShaderResource,
	//StreamOut,
	IndirectArgument,
	CopyDest,
	CopySource,
	ResolveDest,
	ResolveSource,
	RaytracingAccelerationStructure,
	//ShadingRateSource,
	//GenericRead,
	//AllShaderResource,
	Present,
	//Predication,
	//VideoDecodeRead,
	//VideoDecodeWrite,
	//VideoProcessRead,
	//VideoProcessWrite,
	//VideoEncodeRead,
	//VideoEncodeWrite,
};

RhiError      rhiCreateDevice(RhiDevice** outDevice, const RhiDeviceCreateParams& params);
void          rhiDestroyDevice(RhiDevice* device);

void          rhiBeginFrame(RhiDevice* device);
void          rhiEndFrame(RhiDevice* device);
RhiResource*  rhiGetBackBuffer(RhiDevice* device);

RhiCommandList* rhiOpenCommandList(RhiDevice* device, RhiCommandListType type);
void            rhiCloseCommandList(RhiCommandList* commandList);
void            rhiExecuteCommandList(RhiDevice* device, RhiCommandList* commandList);
void            rhiTransitionState(RhiCommandList* commandList, RhiResource* resource, RhiResourceState newState);
void            rhiClearRenderTarget(RhiCommandList* commandList, RhiResource* resource, Vector4f color);
