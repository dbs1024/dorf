// Copyright (c) Darrin Stewart. All rights reserved.
#pragma once

struct RhiDevice;

struct RhiDeviceCreateParams
{
	void* window;
	bool enableDebug;
	bool enableGpuValidation;
	int maxRenderedFrames;
	int backbufferWidth;
	int backbufferHeight;
};

enum class RhiError : unsigned
{
	Ok,
	InvalidArg,
};

RhiError createRhiDevice(RhiDevice** outDevice, const RhiDeviceCreateParams& params);
void     destroyRhiDevice(RhiDevice* device);
