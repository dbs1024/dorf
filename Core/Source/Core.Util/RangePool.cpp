// Copyright (c) Darrin Stewart. All rights reserved.
#include "Core.Util/RangePool.h"

#include <new>

namespace
{
	// Single allocation layout:
	//   [RangePool header][Span freeSpans[maxItems]][padding][items]
	//
	// maxItems span slots is the upper bound on fragmentation: worst case is
	// every other item allocated, yielding floor(maxItems/2) free spans.
	// Spans are kept sorted by offset so coalescing is a single-pass check
	// of the immediate neighbors.

	struct Span
	{
		int offset;
		int count;
	};

	struct RangePool
	{
		int itemStride;
		int maxItems;
		int freeItems;
		int spanCount;
		int alignment;
	};

	size_t alignUp(size_t n, size_t a)
	{
		return (n + a - 1) & ~(a - 1);
	}

	Span* getSpans(RangePool* pool)
	{
		return reinterpret_cast<Span*>(pool + 1);
	}

	char* getItems(RangePool* pool)
	{
		size_t spansEnd   = sizeof(RangePool) + sizeof(Span) * pool->maxItems;
		size_t itemsStart = alignUp(spansEnd, static_cast<size_t>(pool->alignment));
		return reinterpret_cast<char*>(pool) + itemsStart;
	}
}

RangePoolResult createRangePool(RangePoolHandle& outPool, size_t itemSize, int maxItems, size_t alignment)
{
	if (itemSize == 0 || maxItems <= 0)
	{
		outPool = InvalidRangePoolHandle;
		return RangePoolResult::InvalidArg;
	}

	int    itemStride = static_cast<int>(alignUp(itemSize, alignment));
	size_t spansEnd   = sizeof(RangePool) + sizeof(Span) * maxItems;
	size_t itemsStart = alignUp(spansEnd, alignment);
	size_t totalSize  = itemsStart + static_cast<size_t>(itemStride) * maxItems;

	RangePool* pool = static_cast<RangePool*>(::operator new(totalSize, std::align_val_t(alignment)));
	pool->itemStride = itemStride;
	pool->maxItems   = maxItems;
	pool->freeItems  = maxItems;
	pool->spanCount  = 1;
	pool->alignment  = static_cast<int>(alignment);

	getSpans(pool)[0] = {0, maxItems};

	outPool = pool;
	return RangePoolResult::Success;
}

void destroyRangePool(RangePoolHandle pool)
{
	if (!pool)
		return;

	RangePool* p = static_cast<RangePool*>(pool);
	::operator delete(p, std::align_val_t(static_cast<size_t>(p->alignment)));
}

RangePoolResult allocRange(RangeHandle& outRange, RangePoolHandle pool, int count)
{
	if (!pool || count <= 0)
	{
		outRange = InvalidRangeHandle;
		return RangePoolResult::InvalidArg;
	}

	RangePool* p     = static_cast<RangePool*>(pool);
	Span*      spans = getSpans(p);

	for (int i = 0; i < p->spanCount; ++i)
	{
		if (spans[i].count < count)
			continue;

		outRange        = {spans[i].offset, count};
		spans[i].offset += count;
		spans[i].count  -= count;
		p->freeItems    -= count;

		if (spans[i].count == 0)
		{
			for (int j = i; j < p->spanCount - 1; ++j)
				spans[j] = spans[j + 1];
			--p->spanCount;
		}

		return RangePoolResult::Success;
	}

	outRange = InvalidRangeHandle;
	return RangePoolResult::OutOfResources;
}

void freeRange(RangePoolHandle pool, RangeHandle range)
{
	if (!pool)
		return;

	RangePool* p = static_cast<RangePool*>(pool);
	if (range.offset < 0 || range.count <= 0 || range.offset + range.count > p->maxItems)
		return;

	p->freeItems += range.count;

	Span* spans = getSpans(p);

	// Find insertion position, keeping spans sorted by offset.
	int i = 0;
	while (i < p->spanCount && spans[i].offset < range.offset)
		++i;

	bool mergePrev = (i > 0)             && (spans[i - 1].offset + spans[i - 1].count == range.offset);
	bool mergeNext = (i < p->spanCount)  && (range.offset + range.count == spans[i].offset);

	if (mergePrev && mergeNext)
	{
		spans[i - 1].count += range.count + spans[i].count;
		for (int j = i; j < p->spanCount - 1; ++j)
			spans[j] = spans[j + 1];
		--p->spanCount;
	}
	else if (mergePrev)
	{
		spans[i - 1].count += range.count;
	}
	else if (mergeNext)
	{
		spans[i].offset = range.offset;
		spans[i].count += range.count;
	}
	else
	{
		ACE_ASSERT(p->spanCount < p->maxItems);
		for (int j = p->spanCount; j > i; --j)
			spans[j] = spans[j - 1];
		spans[i] = {range.offset, range.count};
		++p->spanCount;
	}
}

void* getRangePtr(RangePoolHandle pool, RangeHandle range)
{
	if (!pool)
		return nullptr;

	RangePool* p = static_cast<RangePool*>(pool);
	if (range.offset < 0 || range.offset >= p->maxItems)
		return nullptr;

	return getItems(p) + range.offset * p->itemStride;
}

int getRangeItemCount(RangePoolHandle pool)
{
	if (!pool)
		return 0;

	RangePool* p = static_cast<RangePool*>(pool);
	return p->maxItems - p->freeItems;
}

int getRangeItemCapacity(RangePoolHandle pool)
{
	if (!pool)
		return 0;

	return static_cast<RangePool*>(pool)->maxItems;
}
