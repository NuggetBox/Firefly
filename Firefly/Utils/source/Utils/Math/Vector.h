#pragma once
#include "Vector2.hpp"
#include "Vector3.hpp"
#include "Vector4.hpp"

namespace Utils
{
	inline static Vector4f Vec3ToVec4(const Vector3f& vec3)
	{
		return { vec3.x, vec3.y, vec3.z, 1 };
	}
	inline static Vector3f Vec4ToVec3(const Vector4f& vec4)
	{
		return { vec4.x, vec4.y, vec4.z};
	}
}