// Copyright (c) Darrin Stewart. All rights reserved.
#include "Core.Memory/SlabAllocator.h"

#include "Core.Base/Assert.h"
#include "Core.Base/Misc.h"
#include <cstring>
#include <windows.h>

namespace
{
#ifdef _DEBUG
	constexpr unsigned char FreedItemPoison     = 0xFD;
	constexpr unsigned char AllocatedItemPoison = 0xFA;
#endif

	struct Slab
	{
		Slab* next;
		Slab* prev;
		void* freeList;
		int   freeCount;
	};

	void listPushFront(Slab*& head, Slab* slab)
	{
		slab->prev = nullptr;
		slab->next = head;
		if (head)
			head->prev = slab;
		head = slab;
	}

	void listRemove(Slab*& head, Slab* slab)
	{
		if (slab->prev)
			slab->prev->next = slab->next;
		else
			head = slab->next;

		if (slab->next)
			slab->next->prev = slab->prev;
	}
}

struct SlabCache
{
	void*  baseAddress;
	size_t reservedSize;
	size_t itemStride;
	size_t slabSize;
	size_t headerSize;
	int    maxItemCount;
	int    pagesPerSlab;
	int    itemsPerSlab;
	int    slabCount;
	int    committedSlabCount;

	Slab*  freeSlabs;
	Slab*  partialSlabs;
	Slab*  fullSlabs;
};

SlabCache* createSlabCache(const SlabCacheParams& params)
{
	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);
	ACE_ASSERT(SlabAllocatorPageSize == systemInfo.dwPageSize);

	size_t itemStride = alignUp<size_t>(params.itemSize, 16);
	size_t slabSize   = static_cast<size_t>(params.pagesPerSlab) * SlabAllocatorPageSize;
	size_t headerSize = alignUp(sizeof(Slab), itemStride);

	int itemsPerSlab = static_cast<int>((slabSize - headerSize) / itemStride);
	ACE_ASSERT(itemsPerSlab > 0);

	int    slabCount = (params.maxItemCount + itemsPerSlab - 1) / itemsPerSlab;
	size_t totalSize = static_cast<size_t>(slabCount) * slabSize;

	void* baseAddress = VirtualAlloc(nullptr, totalSize, MEM_RESERVE, PAGE_READWRITE);
	ACE_ASSERT(baseAddress != nullptr);

	SlabCache* cache = new SlabCache;
	memset(cache, 0, sizeof(*cache));
	cache->baseAddress        = baseAddress;
	cache->reservedSize       = totalSize;
	cache->itemStride         = itemStride;
	cache->slabSize           = slabSize;
	cache->headerSize         = headerSize;
	cache->maxItemCount       = params.maxItemCount;
	cache->pagesPerSlab       = params.pagesPerSlab;
	cache->itemsPerSlab       = itemsPerSlab;
	cache->slabCount          = slabCount;
	cache->committedSlabCount = 0;

	return cache;
}

void destroySlabCache(SlabCache* cache)
{
	ACE_ASSERT(cache->fullSlabs == nullptr && cache->partialSlabs == nullptr);

	VirtualFree(cache->baseAddress, 0, MEM_RELEASE);
	delete cache;
}

void destroySlabCacheUnchecked(SlabCache* cache)
{
	VirtualFree(cache->baseAddress, 0, MEM_RELEASE);
	delete cache;
}

namespace
{
	Slab* commitSlab(SlabCache* cache)
	{
		if (cache->committedSlabCount >= cache->slabCount)
			return nullptr;

		char* slabBase = static_cast<char*>(cache->baseAddress) + static_cast<size_t>(cache->committedSlabCount) * cache->slabSize;
		VirtualAlloc(slabBase, cache->slabSize, MEM_COMMIT, PAGE_READWRITE);
		++cache->committedSlabCount;

		Slab* slab = reinterpret_cast<Slab*>(slabBase);
		slab->next      = nullptr;
		slab->prev      = nullptr;
		slab->freeCount = cache->itemsPerSlab;

		char* items = slabBase + cache->headerSize;
		for (int i = 0; i < cache->itemsPerSlab; ++i)
		{
			char* item = items + static_cast<size_t>(i) * cache->itemStride;
			*reinterpret_cast<void**>(item) = (i + 1 < cache->itemsPerSlab) ? items + static_cast<size_t>(i + 1) * cache->itemStride : nullptr;
#ifdef _DEBUG
			memset(item + sizeof(void*), FreedItemPoison, cache->itemStride - sizeof(void*));
#endif
		}
		slab->freeList = items;

		return slab;
	}
}

void* slabCacheAlloc(SlabCache* cache)
{
	Slab* slab = cache->partialSlabs;

	if (!slab)
	{
		slab = cache->freeSlabs;
		if (slab)
		{
			listRemove(cache->freeSlabs, slab);
			listPushFront(cache->partialSlabs, slab);
		}
	}

	if (!slab)
	{
		slab = commitSlab(cache);
		if (!slab)
			return nullptr;
		listPushFront(cache->partialSlabs, slab);
	}

	void* item = slab->freeList;
	void* next = *reinterpret_cast<void**>(item);

#ifdef _DEBUG
	unsigned char* tail        = static_cast<unsigned char*>(item) + sizeof(void*);
	bool           poisonIntact = true;
	for (size_t i = 0; i < cache->itemStride - sizeof(void*); ++i)
	{
		if (tail[i] != FreedItemPoison)
		{
			poisonIntact = false;
			break;
		}
	}
	ACE_ASSERT(poisonIntact);

	if (next)
	{
		char*  items  = reinterpret_cast<char*>(slab) + cache->headerSize;
		size_t offset = static_cast<size_t>(static_cast<char*>(next) - items);
		ACE_ASSERT(offset < static_cast<size_t>(cache->itemsPerSlab) * cache->itemStride);
		ACE_ASSERT(offset % cache->itemStride == 0);
	}
#endif

	slab->freeList = next;
	--slab->freeCount;

#ifdef _DEBUG
	memset(item, AllocatedItemPoison, cache->itemStride);
#endif

	if (slab->freeCount == 0)
	{
		listRemove(cache->partialSlabs, slab);
		listPushFront(cache->fullSlabs, slab);
	}

	return item;
}

void slabCacheFree(SlabCache* cache, void* ptr)
{
	ACE_ASSERT(ptr != nullptr);

	size_t offset    = static_cast<size_t>(static_cast<char*>(ptr) - static_cast<char*>(cache->baseAddress));
	size_t slabIndex = offset / cache->slabSize;
	Slab*  slab      = reinterpret_cast<Slab*>(static_cast<char*>(cache->baseAddress) + slabIndex * cache->slabSize);

	*reinterpret_cast<void**>(ptr) = slab->freeList;
	slab->freeList = ptr;

#ifdef _DEBUG
	memset(static_cast<char*>(ptr) + sizeof(void*), FreedItemPoison, cache->itemStride - sizeof(void*));
#endif

	bool wasFull = (slab->freeCount == 0);
	++slab->freeCount;

	if (wasFull)
	{
		listRemove(cache->fullSlabs, slab);
		listPushFront(cache->partialSlabs, slab);
	}

	if (slab->freeCount == cache->itemsPerSlab)
	{
		listRemove(cache->partialSlabs, slab);
		listPushFront(cache->freeSlabs, slab);
	}
}
