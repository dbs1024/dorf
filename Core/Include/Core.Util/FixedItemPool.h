// Copyright (c) Darrin Stewart. All rights reserved.
#pragma once

#include "Core.Base/Assert.h"
#include <cstddef>
#include <new>

enum class FixedItemPoolResult : unsigned
{
	Success        = 0,
	OutOfResources = 1,
	InvalidArg     = 2,
};

using FixedItemPoolHandle = void*;
using FixedItemHandle     = int;

constexpr FixedItemPoolHandle InvalidFixedItemPoolHandle = nullptr;
constexpr FixedItemHandle     InvalidFixedItemHandle     = 0;

FixedItemPoolResult createFixedItemPool(FixedItemPoolHandle& outPool, size_t itemSize, int maxItems, size_t alignment = 16);
void                destroyFixedItemPool(FixedItemPoolHandle pool);

void*               allocFixedItem(FixedItemHandle& outItem, FixedItemPoolHandle pool);
void                freeFixedItem(FixedItemPoolHandle pool, FixedItemHandle item);

void*               getFixedItemPtr(FixedItemPoolHandle pool, FixedItemHandle item);

int                 getFixedItemCount(FixedItemPoolHandle pool);
int                 getFixedItemCapacity(FixedItemPoolHandle pool);

// ---------------------------------------------------------------------------

template<typename T>
class FixedItemPoolT
{
public:
	FixedItemPoolT();
	FixedItemPoolT(int maxItems, size_t alignment = 16);
	~FixedItemPoolT();

	FixedItemPoolT(const FixedItemPoolT&)            = delete;
	FixedItemPoolT& operator=(const FixedItemPoolT&) = delete;

	void init(int maxItems, size_t alignment = 16);
	void destroy();

	FixedItemHandle alloc();
	void            free(FixedItemHandle item);
	T*              getPtr(FixedItemHandle item);

private:
	FixedItemPoolHandle m_pool = InvalidFixedItemPoolHandle;
};

template<typename T>
inline FixedItemPoolT<T>::FixedItemPoolT()
{
}

template<typename T>
inline FixedItemPoolT<T>::FixedItemPoolT(int maxItems, size_t alignment)
{
	init(maxItems, alignment);
}

template<typename T>
inline FixedItemPoolT<T>::~FixedItemPoolT()
{
	destroy();
}

template<typename T>
inline void FixedItemPoolT<T>::init(int maxItems, size_t alignment)
{
	FixedItemPoolResult result = createFixedItemPool(m_pool, sizeof(T), maxItems, alignment);
	ACE_ASSERT(result == FixedItemPoolResult::Success);
}

template<typename T>
inline void FixedItemPoolT<T>::destroy()
{
	if (m_pool == InvalidFixedItemPoolHandle)
		return;
	destroyFixedItemPool(m_pool);
	m_pool = InvalidFixedItemPoolHandle;
}

template<typename T>
inline FixedItemHandle FixedItemPoolT<T>::alloc()
{
	FixedItemHandle item;
	void* ptr = allocFixedItem(item, m_pool);
	if (!ptr)
		return InvalidFixedItemHandle;
	::new (ptr) T;
	return item;
}

template<typename T>
inline void FixedItemPoolT<T>::free(FixedItemHandle item)
{
	ACE_ASSERT(item);
	if (!item)
		return;
	getPtr(item)->~T();
	freeFixedItem(m_pool, item);
}

template<typename T>
inline T* FixedItemPoolT<T>::getPtr(FixedItemHandle item)
{
	ACE_ASSERT(item);
	if (!item)
		return nullptr;
	return static_cast<T*>(getFixedItemPtr(m_pool, item));
}
