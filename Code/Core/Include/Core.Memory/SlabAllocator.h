// Copyright (c) Darrin Stewart. All rights reserved.
#pragma once

#include <cstddef>

struct SlabCache;

constexpr size_t SlabAllocatorPageSize = 4096;

struct SlabCacheParams
{
	int    maxItemCount;
	size_t itemSize;
	int    pagesPerSlab;
};

SlabCache* createSlabCache(const SlabCacheParams& params);
void       destroySlabCache(SlabCache* cache);

void*      slabCacheAlloc(SlabCache* cache);
void       slabCacheFree(SlabCache* cache, void* ptr);
