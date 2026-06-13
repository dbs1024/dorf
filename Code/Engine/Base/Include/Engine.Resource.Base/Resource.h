// Copyright (c) Darrin Stewart. All rights reserved.

#pragma once

#include "Core.Base/Types.h"

// Layout of an Engine Resource:
//
// +------------------------------------------
// | ResourceHeader
// +------------------------------------------
// | RelocFixupOffset[0]
// | RelocFixupOffset[1]
// | RelocFixupOffset[2]
// | ...
// | RelocFixupOffset[relocCount - 1]
// +------------------------------------------
// | Resource Data
// |
// |
// |
// +------------------------------------------
//
struct ResourceHeader
{
	u32 relocCount;
	u32 dataSize;
	u32 padding[2];
};
static_assert(sizeof(ResourceHeader) == 16, "EngineResourceHeader must be exactly 16 bytes");

