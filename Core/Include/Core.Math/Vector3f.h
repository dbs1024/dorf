// Copyright (c) Darrin Stewart. All rights reserved.
#pragma once

struct Vector3f
{
	float x;
	float y;
	float z;

	Vector3f operator+(const Vector3f& rhs) const { return { x + rhs.x, y + rhs.y, z + rhs.z }; }
	Vector3f operator-(const Vector3f& rhs) const { return { x - rhs.x, y - rhs.y, z - rhs.z }; }

	Vector3f operator*(float s) const { return { x * s, y * s, z * s }; }

	Vector3f& operator+=(const Vector3f& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
	Vector3f& operator-=(const Vector3f& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
	Vector3f& operator*=(float s)             { x *= s;     y *= s;     z *= s;     return *this; }
};

inline Vector3f operator*(float s, const Vector3f& v) { return v * s; }
