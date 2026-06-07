// Copyright (c) Darrin Stewart. All rights reserved.

#include "Core.ArgParser/ArgParser.h"
#include "Core.Logger/Logger.h"
#include "Core.Vfs/Vfs.h"
#include "Engine.AssetPipeline/AssetPipeline.h"
#include "Engine.Utils.XmlDoc/XmlDoc.h"

#include <cstdio>
#include <cstring>

#include <Windows.h>

using RegisterAssetPipelinesFn = void (*)(AssetPipelineRegistryHandle);

namespace
{
	static const int MaxAssetForgeDlls   = 8;
	static const int MaxAssetBuildParams = 32;

	struct AssetForgeDll
	{
		const char* path;
		HMODULE     module;
	};

	// Resources shared across asset builds.
	struct AssetForgeContext
	{
		Logger*                     logger;
		VfsHandle                   sourceVfs;
		VfsHandle                   outputVfs;
		AssetPipelineRegistryHandle registry;
		AssetForgeDll               dlls[MaxAssetForgeDlls];
		int                         dllCount;
	};

	// Tears down whatever subset of the context has been created so far; every member is safe
	// to destroy in its zeroed state. Must be called on every exit path out of main.
	void destroyAssetForgeContext(AssetForgeContext& context)
	{
		// destroyAssetPipelineRegistry must precede FreeLibrary: pipeline build function pointers
		// and userData stored in the registry point into DLL memory and would dangle otherwise.
		destroyAssetPipelineRegistry(context.registry);

		for (int i = 0; i < context.dllCount; ++i)
			if (context.dlls[i].module)
				FreeLibrary(context.dlls[i].module);

		vfsDestroy(context.outputVfs);
		vfsDestroy(context.sourceVfs);
		destroyLogger(context.logger);
	}

	// Writes the directory portion of path (everything before the last path separator) into
	// outDir, or "." if path has no separator.
	void getParentDirectory(char* outDir, size_t outDirSize, const char* path)
	{
		const char* lastSlash     = strrchr(path, '/');
		const char* lastBackslash = strrchr(path, '\\');
		const char* sep           = lastSlash;
		if (!sep || (lastBackslash && lastBackslash > sep))
			sep = lastBackslash;

		if (!sep)
		{
			snprintf(outDir, outDirSize, ".");
			return;
		}

		size_t length = (size_t)(sep - path);
		if (length >= outDirSize)
			length = outDirSize - 1;

		memcpy(outDir, path, length);
		outDir[length] = '\0';
	}

	enum class AssetBuildOutcome
	{
		Ok,
		Warning,
		Error,
	};

	// Validates one <Asset> element, collects its non-reserved attributes as build params, and
	// dispatches it to the registered pipeline matching its 'type' attribute.
	AssetBuildOutcome buildAssetFromElement(AssetForgeContext& context, XmlDocument* doc, XmlElementHandle element)
	{
		const char* type = getXmlAttributeAsString(doc, element, "type");
		const char* name = getXmlAttributeAsString(doc, element, "name");
		const char* src  = getXmlAttributeAsString(doc, element, "src");

		if (!type || !name || !src)
		{
			aceLog(context.logger, LogLevel::Error, "asset element is missing a required 'type', 'name', or 'src' attribute");
			return AssetBuildOutcome::Error;
		}

		if (validateAssetName(name) != AssetNameValidationResult::Ok)
		{
			aceLog(context.logger, LogLevel::Error, "asset '%s' has an invalid name", name);
			return AssetBuildOutcome::Error;
		}

		AssetPipelineBuildParam params[MaxAssetBuildParams];
		unsigned                paramCount = 0;
		for (XmlAttributeHandle attr = getFirstXmlAttribute(doc, element); attr; attr = getNextXmlAttribute(doc, attr))
		{
			const char* attrName = getXmlAttributeName(doc, attr);
			if (strcmp(attrName, "type") == 0 || strcmp(attrName, "name") == 0 || strcmp(attrName, "src") == 0)
				continue;

			if (paramCount >= MaxAssetBuildParams)
			{
				aceLog(context.logger, LogLevel::Error, "asset '%s' has too many parameters (max %d)", name, MaxAssetBuildParams);
				return AssetBuildOutcome::Error;
			}

			params[paramCount].name  = attrName;
			params[paramCount].value = getXmlAttributeValue(doc, attr);
			++paramCount;
		}

		AssetPipelineBuildContext buildContext;
		memset(&buildContext, 0, sizeof(buildContext));
		buildContext.assetName  = name;
		buildContext.sourcePath = src;
		buildContext.params     = params;
		buildContext.paramCount = paramCount;
		buildContext.logger     = context.logger;
		buildContext.sourceVfs  = context.sourceVfs;
		buildContext.outputVfs  = context.outputVfs;

		AssetPipelineBuildResult buildResult;
		AssetPipelineResult      dispatchResult = buildAsset(&buildResult, context.registry, type, &buildContext);
		if (dispatchResult == AssetPipelineResult::NotFound)
		{
			aceLog(context.logger, LogLevel::Error, "asset '%s' has unrecognized type '%s'", name, type);
			return AssetBuildOutcome::Error;
		}
		if (dispatchResult != AssetPipelineResult::Ok)
		{
			aceLog(context.logger, LogLevel::Error, "asset '%s' failed to dispatch to its pipeline", name);
			return AssetBuildOutcome::Error;
		}

		switch (buildResult)
		{
		case AssetPipelineBuildResult::Ok:      return AssetBuildOutcome::Ok;
		case AssetPipelineBuildResult::Warning: return AssetBuildOutcome::Warning;
		default:                                return AssetBuildOutcome::Error;
		}
	}
}

int main(int argc, char* argv[])
{
	static const ArgDesc argDescs[] =
	{
		{
			.name         = "--dll",
			.flag         = 0,
			.argType      = ArgParserArgType::Auto,
			.action       = ArgParserAction::Append,
			.numArgs      = (unsigned)ArgParserNumArgs::Auto,
			.constant     = nullptr,
			.defaultValue = nullptr,
			.choices      = nullptr,
			.required     = false,
			.help         = nullptr,
			.metavar      = nullptr,
		},
		{
			.name         = "infile",
			.flag         = 0,
			.argType      = ArgParserArgType::Auto,
			.action       = ArgParserAction::Store,
			.numArgs      = (unsigned)ArgParserNumArgs::Auto,
			.constant     = nullptr,
			.defaultValue = nullptr,
			.choices      = nullptr,
			.required     = true,
			.help         = nullptr,
			.metavar      = nullptr,
		},
		{
			.name         = "outpath",
			.flag         = 0,
			.argType      = ArgParserArgType::Auto,
			.action       = ArgParserAction::Store,
			.numArgs      = (unsigned)ArgParserNumArgs::Auto,
			.constant     = nullptr,
			.defaultValue = nullptr,
			.choices      = nullptr,
			.required     = true,
			.help         = nullptr,
			.metavar      = nullptr,
		},
	};

	ArgParserDesc desc;
	desc.description  = "Builds data, preparing it for efficient consumption in a real-time application.";
	desc.argDescs     = argDescs;
	desc.argDescCount = (int)(sizeof(argDescs) / sizeof(argDescs[0]));

	ArgParser parser(desc);
	bool parseOk = parser.parseArgs(argc, (const char**)argv);
	if (parser.shouldPrintHelp())
	{
		printf("%s\n", parser.getHelpMessage());
		return 0;
	}
	if (!parseOk)
	{
		printf("%s\n", parser.getErrorMessage());
		return -1;
	}

	AssetForgeContext context;
	memset(&context, 0, sizeof(context));

	if (!createLogger(&context.logger))
	{
		printf("error: failed to create logger\n");
		destroyAssetForgeContext(context);
		return -1;
	}

	const char* infile  = parser.getStringValue("infile");
	const char* outpath = parser.getStringValue("outpath");

	char sourceRootDir[512];
	getParentDirectory(sourceRootDir, sizeof(sourceRootDir), infile);

	if (vfsCreate(&context.sourceVfs) != VfsResult::Ok || vfsMountDisk(context.sourceVfs, "", sourceRootDir, 0) != VfsResult::Ok)
	{
		printf("error: failed to mount source path '%s'\n", sourceRootDir);
		destroyAssetForgeContext(context);
		return -1;
	}

	if (vfsCreate(&context.outputVfs) != VfsResult::Ok || vfsMountDisk(context.outputVfs, "", outpath, 0) != VfsResult::Ok)
	{
		printf("error: failed to mount output path '%s'\n", outpath);
		destroyAssetForgeContext(context);
		return -1;
	}

	if (createAssetPipelineRegistry(&context.registry) != AssetPipelineResult::Ok)
	{
		printf("error: failed to create asset pipeline registry\n");
		destroyAssetForgeContext(context);
		return -1;
	}

	int dllCount = parser.getValueCount("dll");
	if (dllCount > MaxAssetForgeDlls)
	{
		printf("error: too many --dll arguments (max %d)\n", MaxAssetForgeDlls);
		destroyAssetForgeContext(context);
		return -1;
	}

	context.dllCount = dllCount;
	for (int i = 0; i < dllCount; ++i)
		context.dlls[i].path = parser.getStringValue("dll", i);

	for (int i = 0; i < context.dllCount; ++i)
	{
		AssetForgeDll& dll = context.dlls[i];

		dll.module = LoadLibraryA(dll.path);
		if (!dll.module)
		{
			printf("error: failed to load '%s' (error %lu)\n", dll.path, GetLastError());
			destroyAssetForgeContext(context);
			return -1;
		}

		auto registerFn = (RegisterAssetPipelinesFn)GetProcAddress(dll.module, "assetForgeRegisterPipelines");
		if (!registerFn)
		{
			printf("error: '%s' does not export assetForgeRegisterPipelines\n", dll.path);
			destroyAssetForgeContext(context);
			return -1;
		}

		registerFn(context.registry);
	}

	XmlDocument* assetListDoc = nullptr;
	if (loadXmlDoc(&assetListDoc, infile) != XmlResult::Ok)
	{
		printf("error: failed to load asset list '%s'\n", infile);
		destroyAssetForgeContext(context);
		return -1;
	}

	XmlElementHandle root = getRootXmlElement(assetListDoc);
	if (!root)
	{
		printf("error: asset list '%s' has no root element\n", infile);
		destroyXmlDoc(assetListDoc);
		destroyAssetForgeContext(context);
		return -1;
	}

	int okCount      = 0;
	int warningCount = 0;
	int errorCount   = 0;

	for (XmlElementHandle element = getFirstChildXmlElement(assetListDoc, root, "Asset"); element;
	     element                  = getNextSiblingXmlElement(assetListDoc, element, "Asset"))
	{
		switch (buildAssetFromElement(context, assetListDoc, element))
		{
		case AssetBuildOutcome::Ok:      ++okCount;      break;
		case AssetBuildOutcome::Warning: ++warningCount; break;
		case AssetBuildOutcome::Error:   ++errorCount;   break;
		}
	}

	destroyXmlDoc(assetListDoc);

	printf("AssetForge: %d ok, %d warning(s), %d error(s)\n", okCount, warningCount, errorCount);

	destroyAssetForgeContext(context);
	return (errorCount > 0) ? -1 : 0;
}
