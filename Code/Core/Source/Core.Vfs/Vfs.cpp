// Copyright (c) Darrin Stewart. All rights reserved.
#include "Core.Vfs/Vfs.h"

#include "Core.Memory/FixedItemPool.h"

#include <cstdio>
#include <cstring>
#include <new>

struct VfsMount
{
	char                mountPath[512];
	char                backendPath[512];
	VfsBackendOps       ops;
	void*               backendCtx;
	int                 priority;
	FixedItemPoolHandle filePool;
	FixedItemPoolHandle dirPool;
	FixedItemPoolHandle requestPool;
};

static const unsigned int MaxVfsMounts          = 8;
static const int          MaxVfsFilesPerMount    = 1024;
static const int          MaxVfsDirsPerMount     = 1024;
static const int          MaxVfsRequestsPerMount = 1024;

struct Vfs
{
	FixedItemPoolHandle mountPool;
	FixedItemHandle     mountHandles[MaxVfsMounts];
	unsigned int        mountCount;
};

struct VfsFile
{
	VfsBackendFileHandle backendFile;
	FixedItemPoolHandle  mountPool;
	FixedItemHandle      mountHandle;
	FixedItemHandle      filePoolHandle;
};

struct VfsDir
{
	VfsBackendDirHandle backendDir;
	FixedItemPoolHandle mountPool;
	FixedItemHandle     mountHandle;
	FixedItemHandle     dirPoolHandle;
};

struct VfsRequest
{
	VfsBackendRequestHandle backendRequest;
	FixedItemPoolHandle     mountPool;
	FixedItemHandle         mountHandle;
	FixedItemHandle         requestPoolHandle;
};

static const VfsMount* findMount(const Vfs* vfs, const char* path, const char** outRelativePath,
                                 FixedItemHandle* outMountHandle = nullptr)
{
	const VfsMount* best       = nullptr;
	FixedItemHandle bestHandle = InvalidFixedItemHandle;

	for (unsigned int i = 0; i < vfs->mountCount; ++i)
	{
		const VfsMount* mount = (const VfsMount*)getFixedItemPtr(vfs->mountPool, vfs->mountHandles[i]);
		size_t prefixLen = strlen(mount->mountPath);
		if (strncmp(path, mount->mountPath, prefixLen) != 0)
			continue;

		char next = path[prefixLen];
		if (prefixLen > 0 && next != '\0' && next != '/')
			continue;

		if (best == nullptr || mount->priority > best->priority)
		{
			best       = mount;
			bestHandle = vfs->mountHandles[i];
		}
	}

	if (best != nullptr && outRelativePath != nullptr)
	{
		size_t prefixLen = strlen(best->mountPath);
		*outRelativePath = path + prefixLen;
		if (**outRelativePath == '/')
			(*outRelativePath)++;
	}

	if (outMountHandle != nullptr)
		*outMountHandle = bestHandle;

	return best;
}

VfsResult vfsCreate(VfsHandle* outVfs)
{
	if (outVfs == nullptr)
		return VfsResult::InvalidArg;

	Vfs* vfs = new (std::nothrow) Vfs;
	if (vfs == nullptr)
		return VfsResult::InternalError;

	memset(vfs, 0, sizeof(Vfs));

	if (createFixedItemPool(vfs->mountPool, sizeof(VfsMount), (int)MaxVfsMounts) != FixedItemPoolResult::Success)
	{
		delete vfs;
		return VfsResult::InternalError;
	}

	*outVfs = vfs;
	return VfsResult::Ok;
}

void vfsDestroy(VfsHandle vfs)
{
	if (vfs == nullptr)
		return;

	for (unsigned int i = 0; i < vfs->mountCount; ++i)
	{
		VfsMount* mount = (VfsMount*)getFixedItemPtr(vfs->mountPool, vfs->mountHandles[i]);
		if (mount->ops.unmount != nullptr)
			mount->ops.unmount(mount->backendCtx);
		destroyFixedItemPool(mount->filePool);
		destroyFixedItemPool(mount->dirPool);
		destroyFixedItemPool(mount->requestPool);
	}

	destroyFixedItemPool(vfs->mountPool);

	delete vfs;
}

VfsResult vfsMountCustom(VfsHandle vfs, const char* mountPath, const char* backendPath,
                         const VfsBackendOps* ops, void* backendCtx, int priority)
{
	if (vfs == nullptr || mountPath == nullptr || ops == nullptr)
		return VfsResult::InvalidArg;

	if (vfs->mountCount == MaxVfsMounts)
		return VfsResult::InternalError;

	FixedItemHandle mountHandle = InvalidFixedItemHandle;
	void* ptr = allocFixedItem(mountHandle, vfs->mountPool);
	if (ptr == nullptr)
		return VfsResult::InternalError;

	VfsMount* mount = (VfsMount*)ptr;
	snprintf(mount->mountPath, sizeof(mount->mountPath), "%s", mountPath);
	snprintf(mount->backendPath, sizeof(mount->backendPath), "%s", backendPath != nullptr ? backendPath : "");
	mount->ops        = *ops;
	mount->backendCtx = backendCtx;
	mount->priority   = priority;

	if (mount->ops.mount != nullptr)
	{
		VfsResult result = mount->ops.mount(backendCtx);
		if (result != VfsResult::Ok)
		{
			freeFixedItem(vfs->mountPool, mountHandle);
			return result;
		}
	}

	if (createFixedItemPool(mount->filePool, sizeof(VfsFile), MaxVfsFilesPerMount) != FixedItemPoolResult::Success)
	{
		if (mount->ops.unmount != nullptr)
			mount->ops.unmount(backendCtx);
		freeFixedItem(vfs->mountPool, mountHandle);
		return VfsResult::InternalError;
	}

	if (createFixedItemPool(mount->dirPool, sizeof(VfsDir), MaxVfsDirsPerMount) != FixedItemPoolResult::Success)
	{
		destroyFixedItemPool(mount->filePool);
		if (mount->ops.unmount != nullptr)
			mount->ops.unmount(backendCtx);
		freeFixedItem(vfs->mountPool, mountHandle);
		return VfsResult::InternalError;
	}

	if (createFixedItemPool(mount->requestPool, sizeof(VfsRequest), MaxVfsRequestsPerMount) != FixedItemPoolResult::Success)
	{
		destroyFixedItemPool(mount->filePool);
		destroyFixedItemPool(mount->dirPool);
		if (mount->ops.unmount != nullptr)
			mount->ops.unmount(backendCtx);
		freeFixedItem(vfs->mountPool, mountHandle);
		return VfsResult::InternalError;
	}

	vfs->mountHandles[vfs->mountCount++] = mountHandle;
	return VfsResult::Ok;
}

VfsResult vfsMountZip(VfsHandle vfs, const char* mountPath, const char* zipPath, int priority)
{
	(void)vfs;
	(void)mountPath;
	(void)zipPath;
	(void)priority;
	return VfsResult::NotImplemented;
}

VfsResult vfsMountNetwork(VfsHandle vfs, const char* mountPath, const char* baseUrl, int priority)
{
	(void)vfs;
	(void)mountPath;
	(void)baseUrl;
	(void)priority;
	return VfsResult::NotImplemented;
}

VfsResult vfsUnmount(VfsHandle vfs, const char* mountPath, const char* backendPath)
{
	if (vfs == nullptr || mountPath == nullptr)
		return VfsResult::InvalidArg;

	for (unsigned int i = 0; i < vfs->mountCount; ++i)
	{
		VfsMount* mount = (VfsMount*)getFixedItemPtr(vfs->mountPool, vfs->mountHandles[i]);

		if (strcmp(mount->mountPath, mountPath) != 0)
			continue;

		if (backendPath != nullptr && backendPath[0] != '\0' &&
		    strcmp(mount->backendPath, backendPath) != 0)
			continue;

		if (mount->ops.unmount != nullptr)
			mount->ops.unmount(mount->backendCtx);

		destroyFixedItemPool(mount->filePool);
		destroyFixedItemPool(mount->dirPool);
		destroyFixedItemPool(mount->requestPool);
		freeFixedItem(vfs->mountPool, vfs->mountHandles[i]);
		vfs->mountHandles[i] = vfs->mountHandles[--vfs->mountCount];
		return VfsResult::Ok;
	}

	return VfsResult::NotFound;
}

VfsResult vfsOpenFile(VfsFileHandle* outFile, VfsHandle vfs, const char* path, VfsOpenFlags flags)
{
	if (outFile == nullptr || vfs == nullptr || path == nullptr)
		return VfsResult::InvalidArg;

	const char*     relativePath = nullptr;
	FixedItemHandle mountHandle  = InvalidFixedItemHandle;
	const VfsMount* mount = findMount(vfs, path, &relativePath, &mountHandle);
	if (mount == nullptr)
		return VfsResult::NotFound;

	if (mount->ops.openFile == nullptr)
		return VfsResult::NotSupported;

	VfsBackendFileHandle backendFile = nullptr;
	VfsResult result = mount->ops.openFile(&backendFile, mount->backendCtx, relativePath, flags);
	if (result != VfsResult::Ok)
		return result;

	FixedItemHandle fileHandle = InvalidFixedItemHandle;
	void* ptr = allocFixedItem(fileHandle, mount->filePool);
	if (ptr == nullptr)
	{
		if (mount->ops.closeFile != nullptr)
			mount->ops.closeFile(backendFile);
		return VfsResult::InternalError;
	}

	VfsFile* file = (VfsFile*)ptr;
	file->backendFile = backendFile;
	file->mountPool   = vfs->mountPool;
	file->mountHandle = mountHandle;
	file->filePoolHandle = fileHandle;

	*outFile = file;
	return VfsResult::Ok;
}

VfsResult vfsCloseFile(VfsFileHandle file)
{
	if (file == nullptr)
		return VfsResult::InvalidArg;

	VfsMount* mount = (VfsMount*)getFixedItemPtr(file->mountPool, file->mountHandle);

	VfsResult result = VfsResult::Ok;
	if (mount->ops.closeFile != nullptr)
		result = mount->ops.closeFile(file->backendFile);

	freeFixedItem(mount->filePool, file->filePoolHandle);
	return result;
}

VfsResult vfsReadFile(VfsFileHandle file, void* buffer, unsigned int bytesToRead, unsigned int* outBytesRead)
{
	if (file == nullptr || buffer == nullptr)
		return VfsResult::InvalidArg;

	VfsMount* mount = (VfsMount*)getFixedItemPtr(file->mountPool, file->mountHandle);
	if (mount->ops.readFile == nullptr)
		return VfsResult::NotSupported;

	return mount->ops.readFile(file->backendFile, buffer, bytesToRead, outBytesRead);
}

VfsResult vfsWriteFile(VfsFileHandle file, const void* data, unsigned int bytesToWrite, unsigned int* outBytesWritten)
{
	if (file == nullptr || data == nullptr)
		return VfsResult::InvalidArg;

	VfsMount* mount = (VfsMount*)getFixedItemPtr(file->mountPool, file->mountHandle);
	if (mount->ops.writeFile == nullptr)
		return VfsResult::NotSupported;

	return mount->ops.writeFile(file->backendFile, data, bytesToWrite, outBytesWritten);
}

VfsResult vfsSeekFile(VfsFileHandle file, long long offset, VfsSeekOrigin origin)
{
	if (file == nullptr)
		return VfsResult::InvalidArg;

	VfsMount* mount = (VfsMount*)getFixedItemPtr(file->mountPool, file->mountHandle);
	if (mount->ops.seekFile == nullptr)
		return VfsResult::NotSupported;

	return mount->ops.seekFile(file->backendFile, offset, origin);
}

VfsResult vfsTellFile(VfsFileHandle file, long long* outPosition)
{
	if (file == nullptr || outPosition == nullptr)
		return VfsResult::InvalidArg;

	VfsMount* mount = (VfsMount*)getFixedItemPtr(file->mountPool, file->mountHandle);
	if (mount->ops.tellFile == nullptr)
		return VfsResult::NotSupported;

	return mount->ops.tellFile(file->backendFile, outPosition);
}

VfsResult vfsGetFileSize(VfsFileHandle file, long long* outSize)
{
	if (file == nullptr || outSize == nullptr)
		return VfsResult::InvalidArg;

	VfsMount* mount = (VfsMount*)getFixedItemPtr(file->mountPool, file->mountHandle);
	if (mount->ops.getFileSize == nullptr)
		return VfsResult::NotSupported;

	return mount->ops.getFileSize(file->backendFile, outSize);
}

VfsResult vfsFileExists(VfsHandle vfs, const char* path)
{
	if (vfs == nullptr || path == nullptr)
		return VfsResult::InvalidArg;

	const char* relativePath = nullptr;
	const VfsMount* mount = findMount(vfs, path, &relativePath);
	if (mount == nullptr)
		return VfsResult::NotFound;

	if (mount->ops.fileExists == nullptr)
		return VfsResult::NotSupported;

	return mount->ops.fileExists(mount->backendCtx, relativePath);
}

VfsResult vfsDirExists(VfsHandle vfs, const char* path)
{
	if (vfs == nullptr || path == nullptr)
		return VfsResult::InvalidArg;

	const char* relativePath = nullptr;
	const VfsMount* mount = findMount(vfs, path, &relativePath);
	if (mount == nullptr)
		return VfsResult::NotFound;

	if (mount->ops.dirExists == nullptr)
		return VfsResult::NotSupported;

	return mount->ops.dirExists(mount->backendCtx, relativePath);
}

VfsResult vfsOpenDir(VfsDirHandle* outDir, VfsHandle vfs, const char* path)
{
	if (outDir == nullptr || vfs == nullptr || path == nullptr)
		return VfsResult::InvalidArg;

	const char*     relativePath = nullptr;
	FixedItemHandle mountHandle  = InvalidFixedItemHandle;
	const VfsMount* mount = findMount(vfs, path, &relativePath, &mountHandle);
	if (mount == nullptr)
		return VfsResult::NotFound;

	if (mount->ops.openDir == nullptr)
		return VfsResult::NotSupported;

	VfsBackendDirHandle backendDir = nullptr;
	VfsResult result = mount->ops.openDir(&backendDir, mount->backendCtx, relativePath);
	if (result != VfsResult::Ok)
		return result;

	FixedItemHandle dirHandle = InvalidFixedItemHandle;
	void* ptr = allocFixedItem(dirHandle, mount->dirPool);
	if (ptr == nullptr)
	{
		if (mount->ops.closeDir != nullptr)
			mount->ops.closeDir(backendDir);
		return VfsResult::InternalError;
	}

	VfsDir* dir = (VfsDir*)ptr;
	dir->backendDir  = backendDir;
	dir->mountPool   = vfs->mountPool;
	dir->mountHandle = mountHandle;
	dir->dirPoolHandle = dirHandle;

	*outDir = dir;
	return VfsResult::Ok;
}

VfsResult vfsReadDir(VfsDirHandle dir, VfsDirEntry* outEntry)
{
	if (dir == nullptr || outEntry == nullptr)
		return VfsResult::InvalidArg;

	VfsMount* mount = (VfsMount*)getFixedItemPtr(dir->mountPool, dir->mountHandle);
	if (mount->ops.readDir == nullptr)
		return VfsResult::NotSupported;

	return mount->ops.readDir(dir->backendDir, outEntry);
}

VfsResult vfsCloseDir(VfsDirHandle dir)
{
	if (dir == nullptr)
		return VfsResult::InvalidArg;

	VfsMount* mount = (VfsMount*)getFixedItemPtr(dir->mountPool, dir->mountHandle);

	VfsResult result = VfsResult::Ok;
	if (mount->ops.closeDir != nullptr)
		result = mount->ops.closeDir(dir->backendDir);

	freeFixedItem(mount->dirPool, dir->dirPoolHandle);
	return result;
}

VfsResult vfsReadFileAsync(VfsRequestHandle* outRequest, VfsFileHandle file,
                           void* buffer, unsigned int bytesToRead,
                           VfsReadCallback callback, void* userData)
{
	if (outRequest == nullptr || file == nullptr || buffer == nullptr)
		return VfsResult::InvalidArg;

	VfsMount* mount = (VfsMount*)getFixedItemPtr(file->mountPool, file->mountHandle);
	if (mount->ops.readFileAsync == nullptr)
		return VfsResult::NotSupported;

	VfsBackendRequestHandle backendRequest = nullptr;
	VfsResult result = mount->ops.readFileAsync(&backendRequest, file->backendFile,
	                                             buffer, bytesToRead, callback, userData);
	if (result != VfsResult::Ok)
		return result;

	FixedItemHandle requestHandle = InvalidFixedItemHandle;
	void* ptr = allocFixedItem(requestHandle, mount->requestPool);
	if (ptr == nullptr)
	{
		if (mount->ops.destroyRequest != nullptr)
			mount->ops.destroyRequest(backendRequest);
		return VfsResult::InternalError;
	}

	VfsRequest* request = (VfsRequest*)ptr;
	request->backendRequest = backendRequest;
	request->mountPool      = file->mountPool;
	request->mountHandle    = file->mountHandle;
	request->requestPoolHandle = requestHandle;

	*outRequest = request;
	return VfsResult::Ok;
}

VfsResult vfsPollRequest(VfsRequestHandle request, unsigned int* outBytesRead)
{
	if (request == nullptr || outBytesRead == nullptr)
		return VfsResult::InvalidArg;

	VfsMount* mount = (VfsMount*)getFixedItemPtr(request->mountPool, request->mountHandle);
	if (mount->ops.pollRequest == nullptr)
		return VfsResult::NotSupported;

	return mount->ops.pollRequest(request->backendRequest, outBytesRead);
}

void vfsCancelRequest(VfsRequestHandle request)
{
	if (request == nullptr)
		return;

	VfsMount* mount = (VfsMount*)getFixedItemPtr(request->mountPool, request->mountHandle);
	if (mount->ops.cancelRequest != nullptr)
		mount->ops.cancelRequest(request->backendRequest);
}

void vfsDestroyRequest(VfsRequestHandle request)
{
	if (request == nullptr)
		return;

	VfsMount* mount = (VfsMount*)getFixedItemPtr(request->mountPool, request->mountHandle);
	if (mount->ops.destroyRequest != nullptr)
		mount->ops.destroyRequest(request->backendRequest);

	freeFixedItem(mount->requestPool, request->requestPoolHandle);
}
