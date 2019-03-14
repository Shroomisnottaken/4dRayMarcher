#include "Vector3.h"

float Dot(const Vector3& a, const Vector3& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
float GetLength(const Vector3& a)
{
	return sqrtf(a.x*a.x + a.y*a.y + a.z*a.z);
}

Vector3 Cross(const Vector3& a, const Vector3& b)
{
	return Vector3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

Vector3 Normalize(const Vector3& a)
{
	float l = GetLength(a);
	return Vector3(a.x / l, a.y / l, a.z / l);
}

//c must be betwen 0 an 1
Vector3 Mix(const Vector3& a, const Vector3& b, const float& c)
{
	return a * (1 - c) + b * c;
}