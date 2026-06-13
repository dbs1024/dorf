// Copyright (c) Darrin Stewart. All rights reserved.
#include "Core.Vfs/Vfs.h"

#include "Core.Memory/FixedItemPool.h"

#include <cstdio>
#include <cstring>
#include <io.h>
#include <new>
#include <windows.h>

static const int MaxDiskFiles = 1024;

struct DiskCtx
{
	char                basePath[512];
	FixedItemPoolHandle filePool = InvalidFixedItemPoolHandle;
};

struct DiskFile
{
	FILE*               handle;
	FixedItemPoolHandle pool;
	FixedItemHandle     poolHandle;
};

struct DiskDir
{
	HANDLE           findHandle;
	WIN32_FIND_DATAA findData;
	bool             hasPending;
};

static void buildPath(char* outPath, size_t outSize, const char* basePath, const char* relativePath)
{
	if (relativePath[0] == '\0')
		snprintf(outPath, outSize, "%s", basePath);
	else
		snprintf(outPath, outSize, "%s/%s", basePath, relativePath);
}

// Creates every directory in filePath's parent chain that doesn't already exist, so that
// fopen can create the file itself. Helper precedes its caller (diskOpenFile) — no forward declaration.
static void createParentDirectories(const char* filePath)
{
	char buffer[1024];
	snprintf(buffer, sizeof(buffer), "%s", filePath);

	for (char* p = buffer + 1; *p != '\0'; ++p)
	{
		if (*p == '/' || *p == '\\')
		{
			char separator = *p;
			*p = '\0';
			CreateDirectoryA(buffer, nullptr);
			*p = separator;
		}
	}
}

static void diskUnmount(void* backendCtx)
{
	DiskCtx* ctx = (DiskCtx*)backendCtx;
	destroyFixedItemPool(ctx->filePool);
	delete ctx;
}

static VfsResult diskOpenFile(VfsBackendFileHandle* outFile, void* backendCtx,
                              const char* relativePath, VfsOpenFlags flags)
{
	DiskCtx* ctx = (DiskCtx*)backendCtx;

	char fullPath[1024];
	buildPath(fullPath, sizeof(fullPath), ctx->basePath, relativePath);

	unsigned int flagBits = static_cast<unsigned int>(flags);
	bool read   = (flagBits & static_cast<unsigned int>(VfsOpenFlags::Read))   != 0;
	bool write  = (flagBits & static_cast<unsigned int>(VfsOpenFlags::Write))  != 0;
	bool create = (flagBits & static_cast<unsigned int>(VfsOpenFlags::Create)) != 0;

	const char* mode = "rb";
	if (write && create) mode = read ? "w+b" : "wb";
	else if (write)      mode = read ? "r+b" : "wb";

	if (create)
		createParentDirectories(fullPath);

	FILE* f = nullptr;
	fopen_s(&f, fullPath, mode);
	if (f == nullptr)
		return VfsResult::NotFound;

	FixedItemHandle poolHandle = InvalidFixedItemHandle;
	void* ptr = allocFixedItem(poolHandle, ctx->filePool);
	if (ptr == nullptr)
	{
		fclose(f);
		return VfsResult::InternalError;
	}

	DiskFile* diskFile = (DiskFile*)ptr;
	diskFile->handle     = f;
	diskFile->pool       = ctx->filePool;
	diskFile->poolHandle = poolHandle;

	*outFile = (VfsBackendFileHandle)diskFile;
	return VfsResult::Ok;
}

static VfsResult diskCloseFile(VfsBackendFileHandle file)
{
	DiskFile* diskFile = (DiskFile*)file;
	fclose(diskFile->handle);
	freeFixedItem(diskFile->pool, diskFile->poolHandle);
	return VfsResult::Ok;
}

static VfsResult diskReadFile(VfsBackendFileHandle file, void* buffer,
                              unsigned int bytesToRead, unsigned int* outBytesRead)
{
	DiskFile* diskFile = (DiskFile*)file;
	size_t bytesRead = fread(buffer, 1, bytesToRead, diskFile->handle);
	if (outBytesRead != nullptr)
		*outBytesRead = (unsigned int)bytesRead;

	if (bytesRead == 0 && feof(diskFile->handle))
		return VfsResult::EndOfFile;

	if (bytesRead < bytesToRead && ferror(diskFile->handle))
		return VfsResult::IoError;

	return VfsResult::Ok;
}

static VfsResult diskWriteFile(VfsBackendFileHandle file, const void* data,
                               unsigned int bytesToWrite, unsigned int* outBytesWritten)
{
	DiskFile* diskFile = (DiskFile*)file;
	size_t bytesWritten = fwrite(data, 1, bytesToWrite, diskFile->handle);
	if (outBytesWritten != nullptr)
		*outBytesWritten = (unsigned int)bytesWritten;

	if (bytesWritten < bytesToWrite)
		return VfsResult::IoError;

	return VfsResult::Ok;
}

static VfsResult diskSeekFile(VfsBackendFileHandle file, long long offset, VfsSeekOrigin origin)
{
	DiskFile* diskFile = (DiskFile*)file;

	int whence = SEEK_SET;
	if (origin == VfsSeekOrigin::Current) whence = SEEK_CUR;
	else if (origin == VfsSeekOrigin::End) whence = SEEK_END;

	if (_fseeki64(diskFile->handle, offset, whence) != 0)
		return VfsResult::IoError;

	return VfsResult::Ok;
}

static VfsResult diskTellFile(VfsBackendFileHandle file, long long* outPosition)
{
	DiskFile* diskFile = (DiskFile*)file;
	long long pos = _ftelli64(diskFile->handle);
	if (pos < 0)
		return VfsResult::IoError;

	*outPosition = pos;
	return VfsResult::Ok;
}

static VfsResult diskGetFileSize(VfsBackendFileHandle file, long long* outSize)
{
	DiskFile* diskFile = (DiskFile*)file;
	long long size = _filelengthi64(_fileno(diskFile->handle));
	if (size < 0)
		return VfsResult::IoError;

	*outSize = size;
	return VfsResult::Ok;
}

static VfsResult diskFileExists(void* backendCtx, const char* relativePath)
{
	DiskCtx* ctx = (DiskCtx*)backendCtx;

	char fullPath[1024];
	buildPath(fullPath, sizeof(fullPath), ctx->basePath, relativePath);

	DWORD attribs = GetFileAttributesA(fullPath);
	if (attribs == INVALID_FILE_ATTRIBUTES)
		return VfsResult::NotFound;

	if (attribs & FILE_ATTRIBUTE_DIRECTORY)
		return VfsResult::NotFound;

	return VfsResult::Ok;
}

static VfsResult diskDirExists(void* backendCtx, const char* relativePath)
{
	DiskCtx* ctx = (DiskCtx*)backendCtx;

	char fullPath[1024];
	buildPath(fullPath, sizeof(fullPath), ctx->basePath, relativePath);

	DWORD attribs = GetFileAttributesA(fullPath);
	if (attribs == INVALID_FILE_ATTRIBUTES)
		return VfsResult::NotFound;

	if (!(attribs & FILE_ATTRIBUTE_DIRECTORY))
		return VfsResult::NotFound;

	return VfsResult::Ok;
}

static VfsResult diskOpenDir(VfsBackendDirHandle* outDir, void* backendCtx, const char* relativePath)
{
	DiskCtx* ctx = (DiskCtx*)backendCtx;

	char searchPattern[1024];
	if (relativePath[0] == '\0')
		snprintf(searchPattern, sizeof(searchPattern), "%s/*", ctx->basePath);
	else
		snprintf(searchPattern, sizeof(searchPattern), "%s/%s/*", ctx->basePath, relativePath);

	DiskDir* diskDir = new (std::nothrow) DiskDir;
	if (diskDir == nullptr)
		return VfsResult::InternalError;

	diskDir->findHandle = FindFirstFileA(searchPattern, &diskDir->findData);
	if (diskDir->findHandle == INVALID_HANDLE_VALUE)
	{
		delete diskDir;
		return VfsResult::NotFound;
	}

	diskDir->hasPending = true;
	*outDir = (VfsBackendDirHandle)diskDir;
	return VfsResult::Ok;
}

static VfsResult diskReadDir(VfsBackendDirHandle dir, VfsDirEntry* outEntry)
{
	DiskDir* diskDir = (DiskDir*)dir;

	while (true)
	{
		WIN32_FIND_DATAA* data = nullptr;

		if (diskDir->hasPending)
		{
			data = &diskDir->findData;
			diskDir->hasPending = false;
		}
		else
		{
			if (!FindNextFileA(diskDir->findHandle, &diskDir->findData))
				return VfsResult::EndOfFile;
			data = &diskDir->findData;
		}

		if (strcmp(data->cFileName, ".") == 0 || strcmp(data->cFileName, "..") == 0)
			continue;

		snprintf(outEntry->name, sizeof(outEntry->name), "%s", data->cFileName);
		outEntry->isDirectory = (data->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
		outEntry->size        = ((long long)data->nFileSizeHigh << 32) | data->nFileSizeLow;
		return VfsResult::Ok;
	}
}

static VfsResult diskCloseDir(VfsBackendDirHandle dir)
{
	DiskDir* diskDir = (DiskDir*)dir;
	FindClose(diskDir->findHandle);
	delete diskDir;
	return VfsResult::Ok;
}

VfsResult vfsMountDisk(VfsHandle vfs, const char* mountPath, const char* diskPath, int priority)
{
	if (vfs == nullptr || mountPath == nullptr || diskPath == nullptr)
		return VfsResult::InvalidArg;

	DiskCtx* ctx = new (std::nothrow) DiskCtx;
	if (ctx == nullptr)
		return VfsResult::InternalError;

	snprintf(ctx->basePath, sizeof(ctx->basePath), "%s", diskPath);

	FixedItemPoolResult poolResult = createFixedItemPool(ctx->filePool, sizeof(DiskFile), MaxDiskFiles);
	if (poolResult != FixedItemPoolResult::Success)
	{
		delete ctx;
		return VfsResult::InternalError;
	}

	static const VfsBackendOps k_diskOps =
	{
		nullptr,       // mount
		diskUnmount,
		diskOpenFile,
		diskCloseFile,
		diskReadFile,
		diskWriteFile,
		diskSeekFile,
		diskTellFile,
		diskGetFileSize,
		diskFileExists,
		diskDirExists,
		diskOpenDir,
		diskReadDir,
		diskCloseDir,
		nullptr,       // readFileAsync
		nullptr,       // pollRequest
		nullptr,       // cancelRequest
		nullptr,       // destroyRequest
	};

	VfsResult result = vfsMountCustom(vfs, mountPath, diskPath, &k_diskOps, ctx, priority);
	if (result != VfsResult::Ok)
		delete ctx;

	return result;
}
