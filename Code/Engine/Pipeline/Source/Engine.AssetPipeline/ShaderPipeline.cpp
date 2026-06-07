// Copyright (c) Darrin Stewart. All rights reserved.
#include "ShaderPipeline.h"

#include "Core.Logger/Logger.h"

#include <cstring>

namespace
{
	struct ShaderPipeline
	{
		void* compiler; // placeholder for the HLSL/DXIL shader compiler reference
	};
}

AssetPipelineResult createAndRegisterShaderPipeline(ShaderPipelineHandle* outPipeline, AssetPipelineRegistryHandle registry)
{
	ShaderPipeline* pipeline = new ShaderPipeline;
	memset(pipeline, 0, sizeof(ShaderPipeline));

	const AssetPipelineDesc desc   = { "Shader", shaderPipelineBuild, pipeline };
	AssetPipelineResult     result = registerAssetPipeline(registry, &desc);
	if (result != AssetPipelineResult::Ok)
	{
		delete pipeline;
		*outPipeline = InvalidShaderPipelineHandle;
		return result;
	}

	*outPipeline = pipeline;
	return AssetPipelineResult::Ok;
}

void destroyShaderPipeline(ShaderPipelineHandle pipeline)
{
	delete static_cast<ShaderPipeline*>(pipeline);
}

// Build hook for the "Shader" asset pipeline. Stub: not yet implemented.
AssetPipelineBuildResult shaderPipelineBuild(const AssetPipelineBuildContext* context, void* /*userData*/)
{
	aceLog(context->logger, LogLevel::Error, "Shader pipeline build for asset '%s' is not finished", context->assetName);
	return AssetPipelineBuildResult::Error;
}
