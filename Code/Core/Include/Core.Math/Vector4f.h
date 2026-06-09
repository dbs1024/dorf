// Copyright (c) Darrin Stewart. All rights reserved.
#pragma once

struct Vector4f
{
	float x;
	float y;
	float z;
	float w;

	Vector4f operator+(const Vector4f& rhs) const { return { x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w }; }
	Vector4f operator-(const Vector4f& rhs) const { return { x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w }; }

	Vector4f operator*(float s) const { return { x * s, y * s, z * s, w * s }; }

	Vector4f& operator+=(const Vector4f& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this; }
	Vector4f& operator-=(const Vector4f& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w; return *this; }
	Vector4f& operator*=(float s)             { x *= s;     y *= s;     z *= s;     w *= s;     return *this; }
};

inline Vector4f operator*(float s, const Vector4f& v) { return v * s; }
