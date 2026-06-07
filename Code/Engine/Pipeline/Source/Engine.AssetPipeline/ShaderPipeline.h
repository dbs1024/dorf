// Copyright (c) Darrin Stewart. All rights reserved.
#pragma once

#include "Engine.AssetPipeline/AssetPipeline.h"

// Build hook for the built-in "Shader" asset pipeline. Defined in ShaderPipeline.cpp.
AssetPipelineBuildResult shaderPipelineBuild(const AssetPipelineBuildContext* context, void* userData);
