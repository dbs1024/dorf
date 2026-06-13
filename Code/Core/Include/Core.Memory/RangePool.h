// Copyright (c) Darrin Stewart. All rights reserved.
#pragma once

#include "Core.Base/Assert.h"
#include <cstddef>
#include <new>

enum class RangePoolResult : unsigned
{
	Success        = 0,
	OutOfResources = 1,
	InvalidArg     = 2,
};

struct RangeHandle
{
	int offset;
	int count;
};

using RangePoolHandle = void*;

constexpr RangePoolHandle InvalidRangePoolHandle = nullptr;
constexpr RangeHandle     InvalidRangeHandle     = {-1, 0};

RangePoolResult createRangePool(RangePoolHandle& outPool, size_t itemSize, int maxItems, size_t alignment = 16);
void            destroyRangePool(RangePoolHandle pool);

RangePoolResult allocRange(RangeHandle& outRange, RangePoolHandle pool, int count);
void            freeRange(RangePoolHandle pool, RangeHandle range);

void*           getRangePtr(RangePoolHandle pool, RangeHandle range);

int             getRangeItemCount(RangePoolHandle pool);
int             getRangeItemCapacity(RangePoolHandle pool);

// ---------------------------------------------------------------------------

template<typename T>
class RangePoolT
{
public:
	RangePoolT();
	RangePoolT(int maxItems, size_t alignment = 16);
	~RangePoolT();

	RangePoolT(const RangePoolT&)            = delete;
	RangePoolT& operator=(const RangePoolT&) = delete;

	void init(int maxItems, size_t alignment = 16);
	void destroy();

	RangeHandle alloc(int count);
	void        free(RangeHandle range);
	T*          getPtr(RangeHandle range, int index = 0);

private:
	RangePoolHandle m_pool = InvalidRangePoolHandle;
};

template<typename T>
inline RangePoolT<T>::RangePoolT()
{
}

template<typename T>
inline RangePoolT<T>::RangePoolT(int maxItems, size_t alignment)
{
	init(maxItems, alignment);
}

template<typename T>
inline RangePoolT<T>::~RangePoolT()
{
	destroy();
}

template<typename T>
inline void RangePoolT<T>::init(int maxItems, size_t alignment)
{
	RangePoolResult result = createRangePool(m_pool, sizeof(T), maxItems, alignment);
	ACE_ASSERT(result == RangePoolResult::Success);
}

template<typename T>
inline void RangePoolT<T>::destroy()
{
	if (m_pool == InvalidRangePoolHandle)
		return;
	destroyRangePool(m_pool);
	m_pool = InvalidRangePoolHandle;
}

template<typename T>
inline RangeHandle RangePoolT<T>::alloc(int count)
{
	RangeHandle range;
	RangePoolResult result = allocRange(range, m_pool, count);
	ACE_ASSERT(result == RangePoolResult::Success);
	if (result != RangePoolResult::Success)
		return InvalidRangeHandle;
	T* ptr = getPtr(range);
	for (int i = 0; i < count; ++i)
		::new (ptr + i) T;
	return range;
}

template<typename T>
inline void RangePoolT<T>::free(RangeHandle range)
{
	ACE_ASSERT(range.offset >= 0);
	if (range.offset < 0)
		return;
	T* ptr = getPtr(range);
	for (int i = 0; i < range.count; ++i)
		ptr[i].~T();
	freeRange(m_pool, range);
}

template<typename T>
inline T* RangePoolT<T>::getPtr(RangeHandle range, int index)
{
	ACE_ASSERT(range.offset >= 0);
	if (range.offset < 0)
		return nullptr;
	return static_cast<T*>(getRangePtr(m_pool, range)) + index;
}
