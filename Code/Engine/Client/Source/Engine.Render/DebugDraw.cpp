// Copyright (c) Darrin Stewart. All rights reserved.
#include "Engine.Render/DebugDraw.h"

#include <cstring>

struct DebugDrawContext
{
	RhiDevice* device;
};

DebugDrawContext* debugDrawCreateContext(const DebugDrawCreateParams& params)
{
	DebugDrawContext* context = new DebugDrawContext;
	memset(context, 0, sizeof(*context));
	context->device = params.device;
	return context;
}

void debugDrawDestroyContext(DebugDrawContext* context)
{
	delete context;
}
