// Copyright (c) Darrin Stewart. All rights reserved.
#pragma once

#include "Engine.AssetPipeline/AssetPipeline.h"

using ShaderPipelineHandle = void*;

constexpr ShaderPipelineHandle InvalidShaderPipelineHandle = nullptr;

// Creates a ShaderPipeline and registers it with registry under the "Shader" type. On success,
// outPipeline receives the created handle (the registry owns it from then on and is responsible
// for destroying it via destroyShaderPipeline); on failure, outPipeline receives
// InvalidShaderPipelineHandle and any partially-created state has already been cleaned up.
AssetPipelineResult createAndRegisterShaderPipeline(ShaderPipelineHandle* outPipeline, AssetPipelineRegistryHandle registry);
void                destroyShaderPipeline(ShaderPipelineHandle pipeline);

// Build hook for the built-in "Shader" asset pipeline. Defined in ShaderPipeline.cpp.
AssetPipelineBuildResult shaderPipelineBuild(const AssetPipelineBuildContext* context, void* userData);
