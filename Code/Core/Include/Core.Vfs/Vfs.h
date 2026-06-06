// Copyright (c) Darrin Stewart. All rights reserved.
#pragma once

struct Vfs;
struct VfsFile;
struct VfsDir;
struct VfsRequest;
struct VfsBackendFile;
struct VfsBackendDir;
struct VfsBackendRequest;

typedef Vfs*               VfsHandle;
typedef VfsFile*           VfsFileHandle;
typedef VfsDir*            VfsDirHandle;
typedef VfsRequest*        VfsRequestHandle;
typedef VfsBackendFile*    VfsBackendFileHandle;
typedef VfsBackendDir*     VfsBackendDirHandle;
typedef VfsBackendRequest* VfsBackendRequestHandle;

enum class VfsResult : int
{
	Ok             =  0,
	InternalError  = -1,
	NotFound       = -2,
	NotSupported   = -3,
	IoError        = -4,
	EndOfFile      = -5,
	InvalidArg     = -6,
	NotImplemented = -7,
};

enum class VfsSeekOrigin : unsigned
{
	Begin,
	Current,
	End,
};

enum class VfsOpenFlags : unsigned
{
	Read   = 0x01,
	Write  = 0x02,
	Create = 0x04,
};

struct VfsDirEntry
{
	char      name[512];
	int       isDirectory;
	long long size;
};

typedef void (*VfsReadCallback)(VfsRequestHandle request, VfsResult result, void* userData);

struct VfsBackendOps
{
	VfsResult (*mount)(void* backendCtx);
	void      (*unmount)(void* backendCtx);

	VfsResult (*openFile)(VfsBackendFileHandle* outFile, void* backendCtx, const char* relativePath, VfsOpenFlags flags);
	VfsResult (*closeFile)(VfsBackendFileHandle file);
	VfsResult (*readFile)(VfsBackendFileHandle file, void* buffer, unsigned int bytesToRead, unsigned int* outBytesRead);
	VfsResult (*writeFile)(VfsBackendFileHandle file, const void* data, unsigned int bytesToWrite, unsigned int* outBytesWritten);
	VfsResult (*seekFile)(VfsBackendFileHandle file, long long offset, VfsSeekOrigin origin);
	VfsResult (*tellFile)(VfsBackendFileHandle file, long long* outPosition);
	VfsResult (*getFileSize)(VfsBackendFileHandle file, long long* outSize);

	VfsResult (*fileExists)(void* backendCtx, const char* relativePath);
	VfsResult (*dirExists)(void* backendCtx, const char* relativePath);
	VfsResult (*openDir)(VfsBackendDirHandle* outDir, void* backendCtx, const char* relativePath);
	VfsResult (*readDir)(VfsBackendDirHandle dir, VfsDirEntry* outEntry);
	VfsResult (*closeDir)(VfsBackendDirHandle dir);

	VfsResult (*readFileAsync)(VfsBackendRequestHandle* outRequest, VfsBackendFileHandle file,
	                           void* buffer, unsigned int bytesToRead,
	                           VfsReadCallback callback, void* userData);
	VfsResult (*pollRequest)(VfsBackendRequestHandle request, unsigned int* outBytesRead);
	void      (*cancelRequest)(VfsBackendRequestHandle request);
	void      (*destroyRequest)(VfsBackendRequestHandle request);
};

// Lifecycle
VfsResult vfsCreate(VfsHandle* outVfs);
void      vfsDestroy(VfsHandle vfs);

// Mounting
// backendPath identifies the backend source for vfsUnmount; pass nullptr for custom backends with no path identity.
VfsResult vfsMountCustom(VfsHandle vfs, const char* mountPath, const char* backendPath,
                         const VfsBackendOps* ops, void* backendCtx, int priority);
VfsResult vfsMountDisk(VfsHandle vfs, const char* mountPath, const char* diskPath, int priority);
VfsResult vfsMountZip(VfsHandle vfs, const char* mountPath, const char* zipPath, int priority);
VfsResult vfsMountNetwork(VfsHandle vfs, const char* mountPath, const char* baseUrl, int priority);
// Removes the first mount matching mountPath. If backendPath is non-null and non-empty, also matches on backendPath.
VfsResult vfsUnmount(VfsHandle vfs, const char* mountPath, const char* backendPath);

// File operations
VfsResult vfsOpenFile(VfsFileHandle* outFile, VfsHandle vfs, const char* path, VfsOpenFlags flags);
VfsResult vfsCloseFile(VfsFileHandle file);
VfsResult vfsReadFile(VfsFileHandle file, void* buffer, unsigned int bytesToRead, unsigned int* outBytesRead);
VfsResult vfsWriteFile(VfsFileHandle file, const void* data, unsigned int bytesToWrite, unsigned int* outBytesWritten);
VfsResult vfsSeekFile(VfsFileHandle file, long long offset, VfsSeekOrigin origin);
VfsResult vfsTellFile(VfsFileHandle file, long long* outPosition);
VfsResult vfsGetFileSize(VfsFileHandle file, long long* outSize);

// Directory operations
VfsResult vfsFileExists(VfsHandle vfs, const char* path);
VfsResult vfsDirExists(VfsHandle vfs, const char* path);
VfsResult vfsOpenDir(VfsDirHandle* outDir, VfsHandle vfs, const char* path);
VfsResult vfsReadDir(VfsDirHandle dir, VfsDirEntry* outEntry);
VfsResult vfsCloseDir(VfsDirHandle dir);

// Async operations
VfsResult vfsReadFileAsync(VfsRequestHandle* outRequest, VfsFileHandle file,
                           void* buffer, unsigned int bytesToRead,
                           VfsReadCallback callback, void* userData);
VfsResult vfsPollRequest(VfsRequestHandle request, unsigned int* outBytesRead);
void      vfsCancelRequest(VfsRequestHandle request);
void      vfsDestroyRequest(VfsRequestHandle request);
