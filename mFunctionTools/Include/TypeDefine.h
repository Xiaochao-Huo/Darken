#pragma once

#include "glm.hpp"
#include "half.hpp"

using half_float::half;

namespace dk
{
	typedef bool Bool;

	typedef half Float16;
	typedef float Float32;
	typedef double Float64;

	typedef char Int8;
	typedef short Int16;
	typedef int Int32;
	typedef long long Int64;
	typedef long long Address;

	typedef unsigned char UInt8;
	typedef unsigned char Byte;
	typedef unsigned short UInt16;
	typedef unsigned int UInt32;
	typedef unsigned long long UInt64;

	typedef glm::fvec2 Vector2f;
	typedef glm::fvec3 Vector3f;
	typedef glm::fvec4 Vector4f;
	
	typedef glm::dvec2 Vector2d;
	typedef glm::dvec3 Vector3d;
	typedef glm::dvec4 Vector4d;

	typedef glm::i32vec2 Vector2i;
	typedef glm::i32vec3 Vector3i;
	typedef glm::i32vec4 Vector4i;

	typedef glm::fmat2 Mat2f;
	typedef glm::fmat3 Mat3f;
	typedef glm::fmat4 Mat4f;
	
	typedef glm::dmat2 Mat2d;
	typedef glm::dmat3 Mat3d;
	typedef glm::dmat4 Mat4d;
}

using namespace dk;