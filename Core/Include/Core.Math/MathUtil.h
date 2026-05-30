// Copyright (c) Darrin Stewart. All rights reserved.
#pragma once

#include <bit>
#include <cmath>
#include <cstdint>

// Tests whether two floats are nearly equal using units in the last place (ULP) distance.
// Avoids the need to choose an epsilon; handles varying magnitudes automatically.
// Returns false if either value is NaN.
// Reference: http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
inline bool almostEqual(float x, float y)
{
	constexpr uint32_t kMaxUlps  = 4;
	constexpr uint32_t kSignMask = 0x80000000u;

	if (std::isnan(x) || std::isnan(y))
		return false;

	uint32_t xbits   = std::bit_cast<uint32_t>(x);
	uint32_t ybits   = std::bit_cast<uint32_t>(y);
	uint32_t biasedX = (xbits & kSignMask) ? (~xbits + 1) : (xbits | kSignMask);
	uint32_t biasedY = (ybits & kSignMask) ? (~ybits + 1) : (ybits | kSignMask);
	uint32_t dist    = (biasedX >= biasedY) ? (biasedX - biasedY) : (biasedY - biasedX);
	return dist <= kMaxUlps;
}

inline bool almostEqual(double x, double y)
{
	constexpr uint64_t kMaxUlps  = 4;
	constexpr uint64_t kSignMask = 0x8000000000000000ull;

	if (std::isnan(x) || std::isnan(y))
		return false;

	uint64_t xbits   = std::bit_cast<uint64_t>(x);
	uint64_t ybits   = std::bit_cast<uint64_t>(y);
	uint64_t biasedX = (xbits & kSignMask) ? (~xbits + 1) : (xbits | kSignMask);
	uint64_t biasedY = (ybits & kSignMask) ? (~ybits + 1) : (ybits | kSignMask);
	uint64_t dist    = (biasedX >= biasedY) ? (biasedX - biasedY) : (biasedY - biasedX);
	return dist <= kMaxUlps;
}
