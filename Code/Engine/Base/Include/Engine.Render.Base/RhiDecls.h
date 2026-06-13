// Copyright (c) Darrin Stewart. All rights reserved.
#pragma once

enum class RhiShaderStage : unsigned
{
	Vertex,
	Pixel,
	Compute,
};

enum class RhiFormat : unsigned
{
	R8G8B8A8_Unorm,
	R8G8B8A8_Unorm_Srgb,
};
