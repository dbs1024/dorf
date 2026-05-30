// Copyright (c) Darrin Stewart. All rights reserved.
#include "Core.Util/FixedItemPool.h"

#include <new>

namespace
{
	// Single allocation layout:
	//   [FixedItemPool header][int freeStack[maxItems]][padding][items]
	//
	// The header is 4 ints (16 bytes), naturally int-aligned, so freeStack
	// starts immediately after with no padding needed.  Items are padded to
	// the requested alignment; each item slot is rounded up to that same
	// alignment so every slot satisfies the alignment requirement.

	struct FixedItemPool
	{
		int itemStride;
		int maxItems;
		int freeCount;
		int alignment;
	};

	size_t alignUp(size_t n, size_t a)
	{
		return (n + a - 1) & ~(a - 1);
	}

	int* getFreeStack(FixedItemPool* pool)
	{
		return reinterpret_cast<int*>(pool + 1);
	}

	char* getItems(FixedItemPool* pool)
	{
		size_t freeStackEnd = sizeof(FixedItemPool) + sizeof(int) * pool->maxItems;
		size_t itemsStart   = alignUp(freeStackEnd, static_cast<size_t>(pool->alignment));
		return reinterpret_cast<char*>(pool) + itemsStart;
	}
}

FixedItemPoolResult createFixedItemPool(FixedItemPoolHandle& outPool, size_t itemSize, int maxItems, size_t alignment)
{
	if (itemSize == 0 || maxItems <= 0)
	{
		outPool = InvalidFixedItemPoolHandle;
		return FixedItemPoolResult::InvalidArg;
	}

	int    itemStride   = static_cast<int>(alignUp(itemSize, alignment));
	size_t freeStackEnd = sizeof(FixedItemPool) + sizeof(int) * maxItems;
	size_t itemsStart   = alignUp(freeStackEnd, alignment);
	size_t totalSize    = itemsStart + static_cast<size_t>(itemStride) * maxItems;

	FixedItemPool* pool = static_cast<FixedItemPool*>(::operator new(totalSize, std::align_val_t(alignment)));
	pool->itemStride = itemStride;
	pool->maxItems   = maxItems;
	pool->freeCount  = maxItems;
	pool->alignment  = static_cast<int>(alignment);

	int* freeStack = getFreeStack(pool);
	for (int i = 0; i < maxItems; ++i)
		freeStack[i] = maxItems - 1 - i;

	outPool = pool;
	return FixedItemPoolResult::Success;
}

void destroyFixedItemPool(FixedItemPoolHandle pool)
{
	if (!pool)
		return;

	FixedItemPool* p = static_cast<FixedItemPool*>(pool);
	::operator delete(p, std::align_val_t(static_cast<size_t>(p->alignment)));
}

FixedItemPoolResult allocFixedItem(FixedItemHandle& outItem, FixedItemPoolHandle pool)
{
	if (!pool)
	{
		outItem = InvalidFixedItemHandle;
		return FixedItemPoolResult::InvalidArg;
	}

	FixedItemPool* p = static_cast<FixedItemPool*>(pool);
	if (p->freeCount == 0)
	{
		outItem = InvalidFixedItemHandle;
		return FixedItemPoolResult::OutOfResources;
	}

	int* freeStack = getFreeStack(p);
	outItem = freeStack[--p->freeCount];
	return FixedItemPoolResult::Success;
}

void freeFixedItem(FixedItemPoolHandle pool, FixedItemHandle item)
{
	if (!pool)
		return;

	FixedItemPool* p = static_cast<FixedItemPool*>(pool);
	if (item < 0 || item >= p->maxItems)
		return;

	int* freeStack = getFreeStack(p);
	freeStack[p->freeCount++] = item;
}

void* getFixedItemPtr(FixedItemPoolHandle pool, FixedItemHandle item)
{
	if (!pool)
		return nullptr;

	FixedItemPool* p = static_cast<FixedItemPool*>(pool);
	if (item < 0 || item >= p->maxItems)
		return nullptr;

	return getItems(p) + item * p->itemStride;
}

int getFixedItemCount(FixedItemPoolHandle pool)
{
	if (!pool)
		return 0;

	FixedItemPool* p = static_cast<FixedItemPool*>(pool);
	return p->maxItems - p->freeCount;
}

int getFixedItemCapacity(FixedItemPoolHandle pool)
{
	if (!pool)
		return 0;

	return static_cast<FixedItemPool*>(pool)->maxItems;
}
