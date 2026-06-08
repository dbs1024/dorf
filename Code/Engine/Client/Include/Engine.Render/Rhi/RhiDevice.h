// Copyright (c) Darrin Stewart. All rights reserved.
#pragma once

struct RhiDevice;

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

using RhiCommandListHandle = void*;

RhiError createRhiDevice(RhiDevice** outDevice, const RhiDeviceCreateParams& params);
void     destroyRhiDevice(RhiDevice* device);

void     beginRhiDeviceFrame(RhiDevice* device);
void     endRhiDeviceFrame(RhiDevice* device);

RhiCommandListHandle openRhiCommandList(RhiDevice* device, RhiCommandListType type);
void                 closeRhiCommandList(RhiCommandListHandle commandList);
void                 executeRhiCommandList(RhiDevice* device, RhiCommandListHandle commandList);
