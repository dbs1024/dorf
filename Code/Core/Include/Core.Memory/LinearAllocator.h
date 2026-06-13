// Copyright (c) Darrin Stewart. All rights reserved.
#pragma once

#include <cstddef>

struct LinearAllocPool;

struct LinearAllocPoolParams
{
	size_t initialCapacity;
	size_t maxCapacity;
};

LinearAllocPool* createLinearAllocPool(const LinearAllocPoolParams& params);
void             destroyLinearAllocPool(LinearAllocPool* pool);
void             resetLinearAllocPool(LinearAllocPool* pool);

void*            linearAlloc(LinearAllocPool* pool, size_t size, size_t alignment = 0);
