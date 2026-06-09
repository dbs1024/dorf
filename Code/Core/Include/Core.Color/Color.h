// Copyright (c) Darrin Stewart. All rights reserved.
#pragma once

#include <cstdint>
#include "Core.Math/Vector4f.h"

struct Color
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;

	static constexpr Color black()   { return { 0,   0,   0,   255 }; }
	static constexpr Color white()   { return { 255, 255, 255, 255 }; }
	static constexpr Color red()     { return { 255, 0,   0,   255 }; }
	static constexpr Color green()   { return { 0,   255, 0,   255 }; }
	static constexpr Color blue()    { return { 0,   0,   255, 255 }; }
	static constexpr Color yellow()  { return { 255, 255, 0,   255 }; }
	static constexpr Color cyan()    { return { 0,   255, 255, 255 }; }
	static constexpr Color magenta() { return { 255, 0,   255, 255 }; }
};

inline Vector4f colorToVector4f(Color c)
{
	return { c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f };
}

inline Color colorFromVector4f(Vector4f v)
{
	return { (uint8_t)(v.x * 255.0f), (uint8_t)(v.y * 255.0f), (uint8_t)(v.z * 255.0f), (uint8_t)(v.w * 255.0f) };
}
