// Copyright (c) Darrin Stewart. All rights reserved.
#include "Engine.Render/DebugDraw.h"

#include "Engine.Render/Rhi/RhiDevice.h"

#include <cstring>

struct DebugDrawContext
{
	RhiDevice*  device;
	RhiSampler* sampler;
};

DebugDrawContext* debugDrawCreateContext(const DebugDrawCreateParams& params)
{
	DebugDrawContext* context = new DebugDrawContext;
	memset(context, 0, sizeof(*context));
	context->device = params.device;

	RhiSamplerDesc samplerDesc = {};
	samplerDesc.filter   = RhiFilter::MinMagMipLinear;
	samplerDesc.addressU = RhiTextureAddressMode::Wrap;
	samplerDesc.addressV = RhiTextureAddressMode::Wrap;
	samplerDesc.addressW = RhiTextureAddressMode::Wrap;
	context->sampler = rhiCreateSampler(context->device, samplerDesc);

	return context;
}

void debugDrawDestroyContext(DebugDrawContext* context)
{
	rhiDestroySampler(context->device, context->sampler);
	delete context;
}
