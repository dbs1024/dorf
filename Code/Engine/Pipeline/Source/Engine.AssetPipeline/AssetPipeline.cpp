// Copyright (c) Darrin Stewart. All rights reserved.
#include "Engine.AssetPipeline/AssetPipeline.h"

#include "ShaderPipeline.h"

#include "Core.Memory/SlabAllocator.h"

#include <cstring>

namespace
{
	static const int MaxAssetPipelines  = 1024;
	static const int MaxAssetPipelineTypeLength = 63;

	struct AssetPipelineEntry
	{
		char                 type[MaxAssetPipelineTypeLength + 1];
		AssetPipelineBuildFn build;
		void*                userData;
		AssetPipelineEntry*  next;
	};
}

struct AssetPipelineRegistry
{
	SlabCache*           pool;
	AssetPipelineEntry*  firstEntry;
	AssetPipelineEntry*  lastEntry;
	ShaderPipelineHandle shaderPipeline;
};

namespace
{
	bool isReservedWindowsName(const char* name, size_t length)
	{
		static const char* const reservedNames[] =
		{
			"CON", "PRN", "AUX", "NUL",
			"COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
			"LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9",
		};

		// Reserved names are reserved regardless of any extension, e.g. "CON.txt" is reserved.
		size_t baseLength = length;
		for (size_t i = 0; i < length; ++i)
		{
			if (name[i] == '.')
			{
				baseLength = i;
				break;
			}
		}

		if (baseLength == 0)
			return false;

		for (const char* reserved : reservedNames)
		{
			size_t reservedLength = strlen(reserved);
			if (reservedLength != baseLength)
				continue;

			bool matches = true;
			for (size_t i = 0; i < baseLength; ++i)
			{
				char c = name[i];
				if (c >= 'a' && c <= 'z')
					c = static_cast<char>(c - 'a' + 'A');
				if (c != reserved[i])
				{
					matches = false;
					break;
				}
			}

			if (matches)
				return true;
		}

		return false;
	}

	AssetNameValidationResult validateNameSegment(const char* segment, size_t length)
	{
		if (length == 0)
			return AssetNameValidationResult::EmptyPathSegment;

		if ((length == 1 && segment[0] == '.') || (length == 2 && segment[0] == '.' && segment[1] == '.'))
			return AssetNameValidationResult::InvalidCharacter;

		for (size_t i = 0; i < length; ++i)
		{
			unsigned char c = static_cast<unsigned char>(segment[i]);
			if (c < 0x20)
				return AssetNameValidationResult::InvalidCharacter;

			switch (c)
			{
			case '<': case '>': case ':': case '"':
			case '/': case '\\': case '|': case '?': case '*':
				return AssetNameValidationResult::InvalidCharacter;
			default:
				break;
			}
		}

		char last = segment[length - 1];
		if (last == '.' || last == ' ')
			return AssetNameValidationResult::TrailingDotOrSpace;

		if (isReservedWindowsName(segment, length))
			return AssetNameValidationResult::ReservedName;

		return AssetNameValidationResult::Ok;
	}

	AssetPipelineEntry* findAssetPipelineEntry(AssetPipelineRegistryHandle registry, const char* type)
	{
		for (AssetPipelineEntry* entry = registry->firstEntry; entry != nullptr; entry = entry->next)
		{
			if (strcmp(entry->type, type) == 0)
				return entry;
		}

		return nullptr;
	}

	AssetPipelineResult createAndRegisterBuiltInPipelines(AssetPipelineRegistryHandle registry)
	{
		return createAndRegisterShaderPipeline(&registry->shaderPipeline, registry);
	}

	void destroyBuiltInPipelines(AssetPipelineRegistryHandle registry)
	{
		destroyShaderPipeline(registry->shaderPipeline);
	}
}

AssetPipelineResult createAssetPipelineRegistry(AssetPipelineRegistryHandle* outRegistry)
{
	if (!outRegistry)
		return AssetPipelineResult::InvalidArg;

	AssetPipelineRegistry* registry = new AssetPipelineRegistry;
	memset(registry, 0, sizeof(AssetPipelineRegistry));

	SlabCacheParams poolParams = { MaxAssetPipelines, sizeof(AssetPipelineEntry), 1 };
	registry->pool = createSlabCache(poolParams);

	if (createAndRegisterBuiltInPipelines(registry) != AssetPipelineResult::Ok)
	{
		destroyAssetPipelineRegistry(registry);
		*outRegistry = InvalidAssetPipelineRegistryHandle;
		return AssetPipelineResult::InternalError;
	}

	*outRegistry = registry;
	return AssetPipelineResult::Ok;
}

void destroyAssetPipelineRegistry(AssetPipelineRegistryHandle registry)
{
	if (!registry)
		return;

	destroySlabCacheUnchecked(registry->pool);
	destroyBuiltInPipelines(registry);
	delete registry;
}

AssetPipelineResult registerAssetPipeline(AssetPipelineRegistryHandle registry, const AssetPipelineDesc* desc)
{
	if (!registry || !desc || !desc->type || !desc->build)
		return AssetPipelineResult::InvalidArg;

	size_t typeLength = strlen(desc->type);
	if (typeLength == 0 || typeLength > MaxAssetPipelineTypeLength)
		return AssetPipelineResult::InvalidArg;

	if (findAssetPipelineEntry(registry, desc->type) != nullptr)
		return AssetPipelineResult::DuplicateType;

	void* slot = slabCacheAlloc(registry->pool);
	if (!slot)
		return AssetPipelineResult::OutOfResources;

	AssetPipelineEntry* entry = static_cast<AssetPipelineEntry*>(slot);
	memset(entry, 0, sizeof(AssetPipelineEntry));
	memcpy(entry->type, desc->type, typeLength);
	entry->build    = desc->build;
	entry->userData = desc->userData;

	if (registry->lastEntry)
		registry->lastEntry->next = entry;
	else
		registry->firstEntry = entry;
	registry->lastEntry = entry;

	return AssetPipelineResult::Ok;
}

AssetPipelineResult buildAsset(AssetPipelineBuildResult* outBuildResult, AssetPipelineRegistryHandle registry,
                               const char* type, const AssetPipelineBuildContext* context)
{
	if (!outBuildResult || !registry || !type || !context)
		return AssetPipelineResult::InvalidArg;

	AssetPipelineEntry* entry = findAssetPipelineEntry(registry, type);
	if (!entry)
		return AssetPipelineResult::NotFound;

	*outBuildResult = entry->build(context, entry->userData);
	return AssetPipelineResult::Ok;
}

AssetNameValidationResult validateAssetName(const char* assetName)
{
	if (!assetName || assetName[0] == '\0')
		return AssetNameValidationResult::Empty;

	if (assetName[0] == '/')
		return AssetNameValidationResult::LeadingSlash;

	size_t length     = strlen(assetName);
	size_t segmentStart = 0;
	for (size_t i = 0; i <= length; ++i)
	{
		char c = (i < length) ? assetName[i] : '/';
		if (c == '\\')
			return AssetNameValidationResult::BackslashNotAllowed;

		if (c == '/')
		{
			AssetNameValidationResult segmentResult = validateNameSegment(assetName + segmentStart, i - segmentStart);
			if (segmentResult != AssetNameValidationResult::Ok)
				return segmentResult;

			segmentStart = i + 1;
		}
	}

	return AssetNameValidationResult::Ok;
}
