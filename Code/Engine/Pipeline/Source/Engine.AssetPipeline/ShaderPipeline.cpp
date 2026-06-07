// Copyright (c) Darrin Stewart. All rights reserved.
#include "ShaderPipeline.h"

#include "Core.Logger/Logger.h"

// Build hook for the "Shader" asset pipeline. Stub: not yet implemented.
AssetPipelineBuildResult shaderPipelineBuild(const AssetPipelineBuildContext* context, void* /*userData*/)
{
	aceLog(context->logger, LogLevel::Error, "Shader pipeline build for asset '%s' is not finished", context->assetName);
	return AssetPipelineBuildResult::Error;
}
