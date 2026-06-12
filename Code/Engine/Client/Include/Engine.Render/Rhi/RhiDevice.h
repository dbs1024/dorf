// Copyright (c) Darrin Stewart. All rights reserved.
#pragma once

#include "Core.Math/Vector4f.h"

struct RhiCommandList;
struct RhiDevice;
struct RhiResource;
struct RhiSampler;

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

enum class RhiFilter : unsigned
{
	MinMagMipPoint,
	MinMagPointMipLinear,
	MinPointMagLinearMipPoint,
	MinPointMagMipLinear,
	MinLinearMagMipPoint,
	MinLinearMagPointMipLinear,
	MinMagLinearMipPoint,
	MinMagMipLinear,
	Anisotropic,
	ComparisonMinMagMipPoint,
	ComparisonMinMagPointMipLinear,
	ComparisonMinPointMagLinearMipPoint,
	ComparisonMinPointMagMipLinear,
	ComparisonMinLinearMagMipPoint,
	ComparisonMinLinearMagPointMipLinear,
	ComparisonMinMagLinearMipPoint,
	ComparisonMinMagMipLinear,
	ComparisonAnisotropic,
	MinimumMinMagMipPoint,
	MinimumMinMagPointMipLinear,
	MinimumMinPointMagLinearMipPoint,
	MinimumMinPointMagMipLinear,
	MinimumMinLinearMagMipPoint,
	MinimumMinLinearMagPointMipLinear,
	MinimumMinMagLinearMipPoint,
	MinimumMinMagMipLinear,
	MinimumAnisotropic,
	MaximumMinMagMipPoint,
	MaximumMinMagPointMipLinear,
	MaximumMinPointMagLinearMipPoint,
	MaximumMinPointMagMipLinear,
	MaximumMinLinearMagMipPoint,
	MaximumMinLinearMagPointMipLinear,
	MaximumMinMagLinearMipPoint,
	MaximumMinMagMipLinear,
};

enum class RhiTextureAddressMode : unsigned
{
	Wrap,
	Mirror,
	Clamp,
	Border,
	MirrorOnce,
};

enum class RhiComparisonFunc : unsigned
{
	None,
	Never,
	Less,
	Equal,
	LessEqual,
	Greater,
	NotEqual,
	GreaterEqual,
	Always,
};

struct RhiSamplerDesc
{
	RhiFilter filter;
	RhiTextureAddressMode addressU;
	RhiTextureAddressMode addressV;
	RhiTextureAddressMode addressW;
	float mipLodBias;
	unsigned maxAnisotropy;
	RhiComparisonFunc comparisonFunc;
	Vector4f borderColor;
	float minLod;
	float maxLod;
};

struct RhiDeviceCreateParams
{
	void* window;
	bool enableDebug;
	bool enableGpuValidation;
	unsigned maxRenderedFrames;
	int backbufferWidth;
	int backbufferHeight;
};

RhiError      rhiCreateDevice(RhiDevice** outDevice, const RhiDeviceCreateParams& params);
void          rhiDestroyDevice(RhiDevice* device);

RhiSampler*   rhiCreateSampler(RhiDevice* device, const RhiSamplerDesc& desc);
void          rhiDestroySampler(RhiDevice* device, RhiSampler* sampler);

void          rhiBeginFrame(RhiDevice* device);
void          rhiEndFrame(RhiDevice* device);
RhiResource*  rhiGetBackBuffer(RhiDevice* device);

RhiCommandList* rhiOpenCommandList(RhiDevice* device, RhiCommandListType type);
void            rhiCloseCommandList(RhiCommandList* commandList);
void            rhiExecuteCommandList(RhiCommandList* commandList);
void            rhiTransitionState(RhiCommandList* commandList, RhiResource* resource, RhiResourceState newState);
void            rhiClearRenderTarget(RhiCommandList* commandList, RhiResource* resource, Vector4f color);
