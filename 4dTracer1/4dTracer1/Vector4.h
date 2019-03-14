#pragma once

#include "math.h"

struct Vector4
{
	float x;
	float y;
	float z;
	float w;

	Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

	Vector4 operator+ (const Vector4& a) const
	{
		return Vector4(a.x + x, a.y + y, a.z + z, a.w + w);
	}
	Vector4 operator- (const Vector4& a) const
	{
		return Vector4(x - a.x, y - a.y, z - a.z, w - a.w);
	}
	Vector4 operator* (const float& a) const
	{
		return Vector4(x * a, y * a, z * a, w * a);
	}
	Vector4 operator/ (const float& a) const
	{
		return Vector4(x / a, y / a, z / a, w / a);
	}
	bool operator== (const Vector4& a) const
	{
		return (a.x == x && a.y == y && a.z == z && a.w == w);
	}
	bool operator!= (const Vector4& a) const
	{
		return (a.x != x && a.y != y && a.z != z && a.w != w);
	}
};

float Dot(const Vector4& a, const Vector4& b);
float GetLength(const Vector4& a);
Vector4 Cross(const Vector4& a, const Vector4& b);
Vector4 Normalize(const Vector4& a);
Vector4 Mix(const Vector4& a, const Vector4& b, const float& c);
