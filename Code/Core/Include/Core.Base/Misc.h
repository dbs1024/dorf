// Copyright (c) Darrin Stewart. All rights reserved.

#pragma once

template<typename T>
inline T alignUp(T n, T a)
{
	return (n + a - 1) & ~(a - 1);
}

template<typename T>
inline T clamp(T value, T min, T max)
{
	if (value < min)
		return min;
	if (value > max)
		return max;
	return value;
}
