// Copyright (c) Darrin Stewart. All rights reserved.
#include "Core.Memory/LinearAllocator.h"

#include "Core.Base/Assert.h"
#include "Core.Base/Misc.h"
#include <cstring>
#include <windows.h>

namespace
{
	constexpr size_t LinearAllocatorPageSize = 4096;
}

struct LinearAllocPool
{
	void*  baseAddress;
	size_t reservedSize;
	size_t committedSize;
	size_t offset;
};

LinearAllocPool* createLinearAllocPool(const LinearAllocPoolParams& params)
{
	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);
	ACE_ASSERT(LinearAllocatorPageSize == systemInfo.dwPageSize);

	ACE_ASSERT(params.maxCapacity > 0);
	ACE_ASSERT(params.initialCapacity <= params.maxCapacity);

	size_t reservedSize  = alignUp(params.maxCapacity, LinearAllocatorPageSize);
	size_t committedSize = alignUp(params.initialCapacity, LinearAllocatorPageSize);

	void* baseAddress = VirtualAlloc(nullptr, reservedSize, MEM_RESERVE, PAGE_READWRITE);
	ACE_ASSERT(baseAddress != nullptr);

	if (committedSize > 0)
		VirtualAlloc(baseAddress, committedSize, MEM_COMMIT, PAGE_READWRITE);

	LinearAllocPool* pool = new LinearAllocPool;
	memset(pool, 0, sizeof(*pool));
	pool->baseAddress   = baseAddress;
	pool->reservedSize  = reservedSize;
	pool->committedSize = committedSize;

	return pool;
}

void destroyLinearAllocPool(LinearAllocPool* pool)
{
	VirtualFree(pool->baseAddress, 0, MEM_RELEASE);
	delete pool;
}

void resetLinearAllocPool(LinearAllocPool* pool)
{
	pool->offset = 0;
}

void* linearAlloc(LinearAllocPool* pool, size_t size, size_t alignment)
{
	size_t alignedOffset = (alignment > 0) ? alignUp(pool->offset, alignment) : pool->offset;
	size_t newOffset     = alignedOffset + size;

	if (newOffset > pool->reservedSize)
	{
		ACE_ASSERT(false && "LinearAllocPool is out of memory");
		return nullptr;
	}

	if (newOffset > pool->committedSize)
	{
		size_t newCommittedSize = alignUp(newOffset, LinearAllocatorPageSize);
		size_t commitSize       = newCommittedSize - pool->committedSize;
		char*  commitAddress    = static_cast<char*>(pool->baseAddress) + pool->committedSize;
		VirtualAlloc(commitAddress, commitSize, MEM_COMMIT, PAGE_READWRITE);
		pool->committedSize = newCommittedSize;
	}

	char* base = static_cast<char*>(pool->baseAddress);
	memset(base + pool->offset, 0, alignedOffset - pool->offset);

	pool->offset = newOffset;
	return base + alignedOffset;
}
