// Copyright (c) Darrin Stewart. All rights reserved.
#pragma once

struct DebugDrawContext;
struct RhiDevice;

struct DebugDrawCreateParams
{
	RhiDevice* device;
};

DebugDrawContext* debugDrawCreateContext(const DebugDrawCreateParams& params);
void              debugDrawDestroyContext(DebugDrawContext* context);
