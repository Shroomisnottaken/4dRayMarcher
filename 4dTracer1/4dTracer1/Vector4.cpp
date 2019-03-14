#include "Vector4.h"

float Dot(const Vector4& a, const Vector4& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}
float GetLength(const Vector4& a)
{
	return sqrtf(a.x*a.x + a.y*a.y + a.z*a.z + a.w*a.w);
}

Vector4 Cross(const Vector4& a, const Vector4& b) //not correct! unused;)
{
	return Vector4(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x, 0);
}

Vector4 Normalize(const Vector4& a)
{
	float l = GetLength(a);
	return Vector4(a.x / l, a.y / l, a.z / l, a.w / l);
}

//c must be betwen 0 an 1
Vector4 Mix(const Vector4& a, const Vector4& b, const float& c)
{
	return a * (1 - c) + b * c;
}