// Copyright (c) Darrin Stewart. All rights reserved.
#pragma once

struct Logger;
struct Vfs;

struct AssetPipelineRegistry;
using AssetPipelineRegistryHandle = AssetPipelineRegistry*;

constexpr AssetPipelineRegistryHandle InvalidAssetPipelineRegistryHandle = nullptr;

enum class AssetPipelineResult : int
{
	Ok             =  0,
	InternalError  = -1,
	InvalidArg     = -2,
	OutOfResources = -3,
	DuplicateType  = -4,
	NotFound       = -5,
};

// Outcome of a single pipeline build invocation.
enum class AssetPipelineBuildResult : int
{
	Ok      =  0,
	Warning =  1,
	Error   = -1,
};

// A single pipeline-specific build parameter parsed from an XML asset element's attributes.
struct AssetPipelineBuildParam
{
	const char* name;
	const char* value;
};

// Everything a pipeline needs to build one asset.
struct AssetPipelineBuildContext
{
	const char*                    assetName;   // runtime lookup key; also the relative output path (validated, forward slashes only)
	const char*                    sourcePath;  // path to the source asset, relative to sourceVfs
	const AssetPipelineBuildParam* params;      // pipeline-specific parameters from the XML element's attributes
	unsigned                       paramCount;
	Logger*                        logger;
	Vfs*                           sourceVfs;   // read-only; rooted at the source data root
	Vfs*                           outputVfs;   // write; rooted at the build output root
};

using AssetPipelineBuildFn = AssetPipelineBuildResult (*)(const AssetPipelineBuildContext* context, void* userData);

// Describes one pipeline being registered. userData is passed back to build() unchanged;
// the registrant owns its lifetime.
struct AssetPipelineDesc
{
	const char*          type;
	AssetPipelineBuildFn build;
	void*                userData;
};

// Lifecycle
AssetPipelineResult createAssetPipelineRegistry(AssetPipelineRegistryHandle* outRegistry);
void                destroyAssetPipelineRegistry(AssetPipelineRegistryHandle registry);

// Registration. Fails with DuplicateType if a pipeline with the same type string is already registered.
AssetPipelineResult registerAssetPipeline(AssetPipelineRegistryHandle registry, const AssetPipelineDesc* desc);

// Looks up the pipeline matching type and invokes its build function. Fails with NotFound if no
// pipeline is registered for type; otherwise outBuildResult receives the pipeline's outcome.
AssetPipelineResult buildAsset(AssetPipelineBuildResult* outBuildResult, AssetPipelineRegistryHandle registry,
                               const char* type, const AssetPipelineBuildContext* context);

// Asset name validation. An asset name is a '/'-separated relative path; the last segment is the
// asset's name and the preceding segments form its directory. Every segment must satisfy strict
// Windows filename restrictions, since the name is also used as a relative path into outputVfs.
enum class AssetNameValidationResult : int
{
	Ok                  = 0,
	Empty               = 1,
	LeadingSlash        = 2,
	BackslashNotAllowed = 3,
	EmptyPathSegment    = 4,
	InvalidCharacter    = 5,
	ReservedName        = 6,
	TrailingDotOrSpace  = 7,
};

AssetNameValidationResult validateAssetName(const char* assetName);
