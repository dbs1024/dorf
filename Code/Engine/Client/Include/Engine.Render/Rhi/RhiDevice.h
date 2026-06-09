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

RhiError rhiCreateDevice(RhiDevice** outDevice, const RhiDeviceCreateParams& params);
void     rhiDestroyDevice(RhiDevice* device);

void     rhiBeginFrame(RhiDevice* device);
void     rhiEndFrame(RhiDevice* device);

RhiCommandListHandle rhiOpenCommandList(RhiDevice* device, RhiCommandListType type);
void                 rhiCloseCommandList(RhiCommandListHandle commandList);
void                 rhiExecuteCommandList(RhiDevice* device, RhiCommandListHandle commandList);
