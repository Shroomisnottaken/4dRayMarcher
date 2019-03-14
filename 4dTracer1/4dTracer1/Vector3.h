#pragma once

#include "math.h"

struct Vector3
{
	float x;
	float y;
	float z;

	Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

	Vector3 operator+ (const Vector3& a) const
	{
		return Vector3(a.x + x, a.y + y, a.z + z);
	}
	Vector3 operator- (const Vector3& a) const
	{
		return Vector3(x - a.x, y - a.y, z - a.z);
	}
	Vector3 operator* (const float& a) const
	{
		return Vector3(x * a, y * a, z * a);
	}
	Vector3 operator/ (const float& a) const
	{
		return Vector3(x / a, y / a, z / a);
	}
	bool operator== (const Vector3& a) const
	{
		return (a.x == x && a.y == y && a.z == z);
	}
	bool operator!= (const Vector3& a) const
	{
		return (a.x != x && a.y != y && a.z != z);
	}
};

float Dot(const Vector3& a, const Vector3& b);
float GetLength(const Vector3& a);
Vector3 Cross(const Vector3& a, const Vector3& b);
Vector3 Normalize(const Vector3& a);
Vector3 Mix(const Vector3& a, const Vector3& b, const float& c);
