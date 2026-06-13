// Copyright (c) Darrin Stewart. All rights reserved.
#include "ShaderPipeline.h"

#include "Core.Logger/Logger.h"
#include "Core.Vfs/Vfs.h"
#include "Engine.Render.Base/RhiDecls.h"

#include <cstdio>
#include <cstring>
#include <new>

#include <Windows.h>
#include <dxcapi.h>

namespace
{
	static const int MaxShaderCompilerArgs    = 128;
	static const int MaxShaderArgStringLength = 512;

	struct ShaderPipeline
	{
		HMODULE        dxcModule;
		IDxcCompiler3* dxcCompiler;
		IDxcUtils*     dxcUtils;
	};

	struct ShaderBuildParameters
	{
		RhiShaderStage stage;
		const char*    entrypoint;
	};

	// On-disk header for a ".sbd" (shader binary data) file: stage, byte code size, then padding.
	// The compiled byte code follows immediately after this header.
	struct ShaderBinaryHeader
	{
		unsigned int stage;
		unsigned int byteCodeSize;
		unsigned int padding[2];
	};
	static_assert(sizeof(ShaderBinaryHeader) == 16, "ShaderBinaryHeader must be exactly 16 bytes");

	// Helper precedes its caller (shaderPipelineBuild) — no forward declaration.
	bool gatherShaderBuildParameters(ShaderBuildParameters& outParams, const AssetPipelineBuildContext* context)
	{
		const char* stage      = nullptr;
		const char* entrypoint = nullptr;

		for (unsigned i = 0; i < context->paramCount; ++i)
		{
			const AssetPipelineBuildParam& param = context->params[i];

			if (strcmp(param.name, "stage") == 0)
				stage = param.value;
			else if (strcmp(param.name, "entry") == 0)
				entrypoint = param.value;
		}

		if (!stage)
		{
			aceLog(context->logger, LogLevel::Error, "shader asset '%s' is missing required parameter 'stage'", context->assetName);
			return false;
		}

		if (!entrypoint)
		{
			aceLog(context->logger, LogLevel::Error, "shader asset '%s' is missing required parameter 'entry'", context->assetName);
			return false;
		}

		if (strcmp(stage, "vs") == 0)
			outParams.stage = RhiShaderStage::Vertex;
		else if (strcmp(stage, "ps") == 0)
			outParams.stage = RhiShaderStage::Pixel;
		else if (strcmp(stage, "cs") == 0)
			outParams.stage = RhiShaderStage::Compute;
		else
		{
			aceLog(context->logger, LogLevel::Error, "shader asset '%s' has unrecognized 'stage' value '%s'", context->assetName, stage);
			return false;
		}

		outParams.entrypoint = entrypoint;

		return true;
	}

	// Reads sourcePath from sourceVfs into a heap-allocated buffer. On success, the caller owns
	// outBytes and must release it with delete[]. Helper precedes its caller (shaderPipelineBuild)
	// — no forward declaration.
	bool readShaderSourceBytes(unsigned char*& outBytes, unsigned int& outByteCount, const AssetPipelineBuildContext* context)
	{
		outBytes     = nullptr;
		outByteCount = 0;

		VfsFileHandle sourceFile = nullptr;
		if (vfsOpenFile(&sourceFile, context->sourceVfs, context->sourcePath, VfsOpenFlags::Read) != VfsResult::Ok)
		{
			aceLog(context->logger, LogLevel::Error, "shader asset '%s' failed to open source file '%s'", context->assetName, context->sourcePath);
			return false;
		}

		long long sourceSize = 0;
		if (vfsGetFileSize(sourceFile, &sourceSize) != VfsResult::Ok || sourceSize < 0)
		{
			aceLog(context->logger, LogLevel::Error, "shader asset '%s' failed to get the size of source file '%s'", context->assetName, context->sourcePath);
			vfsCloseFile(sourceFile);
			return false;
		}

		unsigned char* bytes = new (std::nothrow) unsigned char[sourceSize];
		if (!bytes)
		{
			aceLog(context->logger, LogLevel::Error, "shader asset '%s' failed to allocate %lld bytes for source file '%s'", context->assetName, sourceSize, context->sourcePath);
			vfsCloseFile(sourceFile);
			return false;
		}

		unsigned int bytesRead  = 0;
		VfsResult    readResult = vfsReadFile(sourceFile, bytes, static_cast<unsigned int>(sourceSize), &bytesRead);
		vfsCloseFile(sourceFile);

		if (readResult != VfsResult::Ok || bytesRead != static_cast<unsigned int>(sourceSize))
		{
			aceLog(context->logger, LogLevel::Error, "shader asset '%s' failed to read source file '%s'", context->assetName, context->sourcePath);
			delete[] bytes;
			return false;
		}

		outBytes     = bytes;
		outByteCount = bytesRead;

		return true;
	}

	// Fills outArgs (capacity MaxShaderCompilerArgs) with the DXC compiler arguments to use for
	// params and context. outSourceName/outEntrypoint are wide-string conversion scratch buffers
	// owned by the caller; outArgs entries point into them, so they must outlive outArgs' use.
	// Helper precedes its caller (shaderPipelineBuild) — no forward declaration.
	bool buildCompilerArgs(const wchar_t** outArgs, unsigned& outArgCount,
	                       wchar_t outSourceName[MaxShaderArgStringLength], wchar_t outEntrypoint[MaxShaderArgStringLength],
	                       const ShaderBuildParameters& params, const AssetPipelineBuildContext* context)
	{
		if (MultiByteToWideChar(CP_UTF8, 0, context->sourcePath, -1, outSourceName, MaxShaderArgStringLength) <= 0 ||
		    MultiByteToWideChar(CP_UTF8, 0, params.entrypoint, -1, outEntrypoint, MaxShaderArgStringLength) <= 0)
		{
			aceLog(context->logger, LogLevel::Error, "shader asset '%s' has a source path or entry point too long to convert to a wide string", context->assetName);
			return false;
		}

		unsigned count = 0;

		// The source file name comes first so DXC can embed it for later PDB matching.
		outArgs[count++] = outSourceName;

		outArgs[count++] = L"-E";
		outArgs[count++] = outEntrypoint;

		outArgs[count++] = DXC_ARG_WARNINGS_ARE_ERRORS;
		outArgs[count++] = DXC_ARG_SKIP_OPTIMIZATIONS;
		outArgs[count++] = DXC_ARG_DEBUG;
		outArgs[count++] = DXC_ARG_DEBUG_NAME_FOR_BINARY;

		outArgs[count++] = L"-T";
		switch (params.stage)
		{
		case RhiShaderStage::Vertex:  outArgs[count++] = L"vs_6_0"; break;
		case RhiShaderStage::Pixel:   outArgs[count++] = L"ps_6_0"; break;
		case RhiShaderStage::Compute: outArgs[count++] = L"cs_6_0"; break;
		}

		outArgCount = count;

		return true;
	}

	// Writes outDir as the directory portion of path (everything before the final '/'), or an
	// empty string if path has no directory component. Helper precedes its caller (writeShaderPdb)
	// — no forward declaration.
	void getAssetDirectory(char* outDir, size_t outDirSize, const char* path)
	{
		const char* lastSlash = strrchr(path, '/');
		if (!lastSlash)
		{
			outDir[0] = '\0';
			return;
		}

		size_t length = (size_t)(lastSlash - path);
		if (length >= outDirSize)
			length = outDirSize - 1;

		memcpy(outDir, path, length);
		outDir[length] = '\0';
	}

	// Writes the PDB output (if any) from result to outputVfs, using the file name DXC suggests
	// so the compiled object (built with -Zsb) can be matched to its pdb later. The pdb is placed
	// alongside the compiled shader binary, in the asset's subfolder of the output. Helper precedes
	// its caller (shaderPipelineBuild) — no forward declaration.
	bool writeShaderPdb(IDxcResult* result, const AssetPipelineBuildContext* context)
	{
		if (!result->HasOutput(DXC_OUT_PDB))
			return true;

		IDxcBlob*     pdbBlob = nullptr;
		IDxcBlobWide* pdbName = nullptr;
		if (FAILED(result->GetOutput(DXC_OUT_PDB, _uuidof(IDxcBlob), reinterpret_cast<void**>(&pdbBlob), &pdbName)) || !pdbBlob || !pdbName)
		{
			aceLog(context->logger, LogLevel::Error, "shader asset '%s' failed to retrieve its compiled pdb", context->assetName);
			if (pdbName)
				pdbName->Release();
			if (pdbBlob)
				pdbBlob->Release();
			return false;
		}

		bool ok = true;
		char pdbFileName[MaxShaderArgStringLength];

		if (WideCharToMultiByte(CP_UTF8, 0, pdbName->GetStringPointer(), -1, pdbFileName, MaxShaderArgStringLength, nullptr, nullptr) <= 0)
		{
			aceLog(context->logger, LogLevel::Error, "shader asset '%s' has a pdb name too long to convert to a narrow string", context->assetName);
			ok = false;
		}

		char pdbPath[MaxShaderArgStringLength];
		if (ok)
		{
			char assetDir[MaxShaderArgStringLength];
			getAssetDirectory(assetDir, sizeof(assetDir), context->assetName);

			if (assetDir[0] != '\0')
				snprintf(pdbPath, sizeof(pdbPath), "%s/%s", assetDir, pdbFileName);
			else
				snprintf(pdbPath, sizeof(pdbPath), "%s", pdbFileName);
		}

		if (ok)
		{
			const VfsOpenFlags writeFlags = static_cast<VfsOpenFlags>(static_cast<unsigned>(VfsOpenFlags::Write) | static_cast<unsigned>(VfsOpenFlags::Create));

			VfsFileHandle pdbFile = nullptr;
			if (vfsOpenFile(&pdbFile, context->outputVfs, pdbPath, writeFlags) != VfsResult::Ok)
			{
				aceLog(context->logger, LogLevel::Error, "shader asset '%s' failed to open pdb output '%s'", context->assetName, pdbPath);
				ok = false;
			}
			else
			{
				unsigned int bytesWritten = 0;
				if (vfsWriteFile(pdbFile, pdbBlob->GetBufferPointer(), static_cast<unsigned int>(pdbBlob->GetBufferSize()), &bytesWritten) != VfsResult::Ok ||
				    bytesWritten != static_cast<unsigned int>(pdbBlob->GetBufferSize()))
				{
					aceLog(context->logger, LogLevel::Error, "shader asset '%s' failed to write pdb '%s'", context->assetName, pdbPath);
					ok = false;
				}

				vfsCloseFile(pdbFile);
			}
		}

		pdbName->Release();
		pdbBlob->Release();

		return ok;
	}

	// Writes the compiled byte code from result to outputVfs as "<assetName>.sbd": a 16-byte
	// ShaderBinaryHeader followed immediately by the raw byte code. Helper precedes its caller
	// (shaderPipelineBuild) — no forward declaration.
	bool writeCompiledShaderBinary(IDxcResult* result, const ShaderBuildParameters& params, const AssetPipelineBuildContext* context)
	{
		IDxcBlob* objectBlob = nullptr;
		if (FAILED(result->GetOutput(DXC_OUT_OBJECT, _uuidof(IDxcBlob), reinterpret_cast<void**>(&objectBlob), nullptr)) || !objectBlob)
		{
			aceLog(context->logger, LogLevel::Error, "shader asset '%s' failed to retrieve its compiled byte code", context->assetName);
			if (objectBlob)
				objectBlob->Release();
			return false;
		}

		ShaderBinaryHeader header;
		memset(&header, 0, sizeof(header));
		header.stage        = static_cast<unsigned int>(params.stage);
		header.byteCodeSize = static_cast<unsigned int>(objectBlob->GetBufferSize());

		char binaryPath[MaxShaderArgStringLength];
		snprintf(binaryPath, sizeof(binaryPath), "%s.sbd", context->assetName);

		bool ok = true;
		const VfsOpenFlags writeFlags = static_cast<VfsOpenFlags>(static_cast<unsigned>(VfsOpenFlags::Write) | static_cast<unsigned>(VfsOpenFlags::Create));

		VfsFileHandle binaryFile = nullptr;
		if (vfsOpenFile(&binaryFile, context->outputVfs, binaryPath, writeFlags) != VfsResult::Ok)
		{
			aceLog(context->logger, LogLevel::Error, "shader asset '%s' failed to open binary output '%s'", context->assetName, binaryPath);
			ok = false;
		}
		else
		{
			unsigned int headerBytesWritten = 0;
			unsigned int codeBytesWritten   = 0;

			if (vfsWriteFile(binaryFile, &header, sizeof(header), &headerBytesWritten) != VfsResult::Ok || headerBytesWritten != sizeof(header) ||
			    vfsWriteFile(binaryFile, objectBlob->GetBufferPointer(), header.byteCodeSize, &codeBytesWritten) != VfsResult::Ok || codeBytesWritten != header.byteCodeSize)
			{
				aceLog(context->logger, LogLevel::Error, "shader asset '%s' failed to write binary '%s'", context->assetName, binaryPath);
				ok = false;
			}

			vfsCloseFile(binaryFile);
		}

		objectBlob->Release();

		return ok;
	}
}

AssetPipelineResult createAndRegisterShaderPipeline(ShaderPipelineHandle* outPipeline, AssetPipelineRegistryHandle registry)
{
	ShaderPipeline* pipeline = new ShaderPipeline;
	memset(pipeline, 0, sizeof(ShaderPipeline));

	pipeline->dxcModule = LoadLibraryA("dxcompiler.dll");

	DxcCreateInstanceProc createInstance = pipeline->dxcModule
		? (DxcCreateInstanceProc)GetProcAddress(pipeline->dxcModule, "DxcCreateInstance")
		: nullptr;

	if (!createInstance ||
	    FAILED(createInstance(CLSID_DxcCompiler, _uuidof(IDxcCompiler3), reinterpret_cast<void**>(&pipeline->dxcCompiler))) ||
	    FAILED(createInstance(CLSID_DxcUtils, _uuidof(IDxcUtils), reinterpret_cast<void**>(&pipeline->dxcUtils))))
	{
		destroyShaderPipeline(pipeline);
		*outPipeline = InvalidShaderPipelineHandle;
		return AssetPipelineResult::InternalError;
	}

	const AssetPipelineDesc desc   = { "Shader", shaderPipelineBuild, pipeline };
	AssetPipelineResult     result = registerAssetPipeline(registry, &desc);
	if (result != AssetPipelineResult::Ok)
	{
		destroyShaderPipeline(pipeline);
		*outPipeline = InvalidShaderPipelineHandle;
		return result;
	}

	*outPipeline = pipeline;
	return AssetPipelineResult::Ok;
}

void destroyShaderPipeline(ShaderPipelineHandle pipeline)
{
	ShaderPipeline* shaderPipeline = static_cast<ShaderPipeline*>(pipeline);

	if (shaderPipeline->dxcCompiler)
		shaderPipeline->dxcCompiler->Release();
	if (shaderPipeline->dxcUtils)
		shaderPipeline->dxcUtils->Release();
	if (shaderPipeline->dxcModule)
		FreeLibrary(shaderPipeline->dxcModule);

	delete shaderPipeline;
}

// Build hook for the "Shader" asset pipeline: compiles the source HLSL with DXC and writes the
// compiled byte code (".sbd") and pdb to outputVfs.
AssetPipelineBuildResult shaderPipelineBuild(const AssetPipelineBuildContext* context, void* userData)
{
	ShaderPipeline* pipeline = static_cast<ShaderPipeline*>(userData);

	ShaderBuildParameters params;
	if (!gatherShaderBuildParameters(params, context))
		return AssetPipelineBuildResult::Error;

	unsigned char* sourceBytes     = nullptr;
	unsigned int   sourceByteCount = 0;
	if (!readShaderSourceBytes(sourceBytes, sourceByteCount, context))
		return AssetPipelineBuildResult::Error;

	const DxcBuffer sourceBuffer = { sourceBytes, sourceByteCount, DXC_CP_ACP };

	const wchar_t* compilerArgs[MaxShaderCompilerArgs];
	unsigned       compilerArgCount = 0;
	wchar_t        sourceNameWide[MaxShaderArgStringLength];
	wchar_t        entrypointWide[MaxShaderArgStringLength];
	if (!buildCompilerArgs(compilerArgs, compilerArgCount, sourceNameWide, entrypointWide, params, context))
	{
		delete[] sourceBytes;
		return AssetPipelineBuildResult::Error;
	}

	IDxcIncludeHandler* includeHandler = nullptr;
	if (FAILED(pipeline->dxcUtils->CreateDefaultIncludeHandler(&includeHandler)))
	{
		aceLog(context->logger, LogLevel::Error, "shader asset '%s' failed to create a default include handler", context->assetName);
		delete[] sourceBytes;
		return AssetPipelineBuildResult::Error;
	}

	IDxcResult* compileResult = nullptr;
	HRESULT     compileHr     = pipeline->dxcCompiler->Compile(&sourceBuffer, compilerArgs, compilerArgCount, includeHandler,
	                                                            _uuidof(IDxcResult), reinterpret_cast<void**>(&compileResult));
	if (FAILED(compileHr) || !compileResult)
	{
		aceLog(context->logger, LogLevel::Error, "shader asset '%s' failed to invoke the shader compiler (hr=0x%08X)", context->assetName, compileHr);
		if (compileResult)
			compileResult->Release();
		includeHandler->Release();
		delete[] sourceBytes;
		return AssetPipelineBuildResult::Error;
	}

	HRESULT compileStatus = S_OK;
	compileResult->GetStatus(&compileStatus);

	if (FAILED(compileStatus))
	{
		IDxcBlobUtf8* errors = nullptr;
		compileResult->GetOutput(DXC_OUT_ERRORS, _uuidof(IDxcBlobUtf8), reinterpret_cast<void**>(&errors), nullptr);

		if (errors && errors->GetStringLength() > 0)
			aceLog(context->logger, LogLevel::Error, "shader asset '%s' failed to compile:\n%s", context->assetName, errors->GetStringPointer());
		else
			aceLog(context->logger, LogLevel::Error, "shader asset '%s' failed to compile (hr=0x%08X)", context->assetName, compileStatus);

		if (errors)
			errors->Release();

		compileResult->Release();
		includeHandler->Release();
		delete[] sourceBytes;
		return AssetPipelineBuildResult::Error;
	}

	if (!writeShaderPdb(compileResult, context))
	{
		compileResult->Release();
		includeHandler->Release();
		delete[] sourceBytes;
		return AssetPipelineBuildResult::Error;
	}

	if (!writeCompiledShaderBinary(compileResult, params, context))
	{
		compileResult->Release();
		includeHandler->Release();
		delete[] sourceBytes;
		return AssetPipelineBuildResult::Error;
	}

	compileResult->Release();
	includeHandler->Release();
	delete[] sourceBytes;

	return AssetPipelineBuildResult::Ok;
}
