/***************************************************************************
* SMathTypes.h
*/

#pragma once

#include "SConfig.h"

/***************************************************************************
* Vectors
*/

// Vector 2D
struct SVector2
{
	float x;
	float y;

	bool operator==(const SVector2&) const = default;
};

// Vector 3D
struct SVector3 : public SVector2
{
	float z;

	bool operator==(const SVector3&) const = default;
};

// Vector 4D
struct SVector4 : public SVector3
{
	float w;

	bool operator==(const SVector4&) const = default;
};

namespace SConst
{
	static const SVector2 ZeroSVector2 = SVector2{};
	static const SVector3 ZeroSVector3 = SVector3{};
	static const SVector4 ZeroSVector4 = SVector4{};
}

#if USING_GLMATH
#include "glm/glm.hpp"

namespace SConvert
{
	inline SVector2 ToVector2(const glm::vec2& v)
	{
		return SVector2{ v.x, v.y };
	}

	inline SVector3 ToVector3(const glm::vec3& v)
	{
		return SVector3{ v.x, v.y, v.z };
	}

	inline SVector4 ToVector4(const glm::vec4& v)
	{
		return SVector4{ v.x, v.y, v.z, v.w };
	}
}

#endif // USING_GLMATH

#if USING_EIGEN
#include "Eigen/Dense"

namespace SConvert
{
	inline SVector2 ToVector2(const Eigen::Vector2<float>& v)
	{
		return SVector2{ v.x(), v.y() };
	}

	inline SVector3 ToVector3(const Eigen::Vector3<float>& v)
	{
		return SVector3{ v.x(), v.y(), v.z() };
	}

	inline SVector4 ToVector4(const Eigen::Vector4<float>& v)
	{
		return SVector4{ v.x(), v.y(), v.z(), v.w() };
	}
}

#endif // USING_EIGEN


/***************************************************************************
* Point & Size
*/

// Point 2D
struct SPoint2
{
	std::int32_t x;
	std::int32_t y;

	bool operator==(const SPoint2&) const = default;
};

// Point 3D
struct SPoint3 : public SPoint2
{
	std::int32_t z;

	bool operator==(const SPoint3&) const = default;
};

using SSize3 = SPoint3;

// Size 2D
struct SSize2
{
	std::int32_t width;
	std::int32_t height;

	bool operator==(const SSize2&) const = default;
};

namespace SConst
{
	static const SPoint2 ZeroSPoint2 = SPoint2{};
	static const SPoint3 ZeroSPoint3 = SPoint3{};
	static const SSize2 ZeroSSize2 = SSize2{};
	static const SSize3 ZeroSSize3 = SSize3{};
}


/***************************************************************************
* Matrix
*/

// Matrix 3x3
struct SMatrix3
{
	float m[9];
};

// Matrix 4x4
struct SMatrix4
{
	float m[16];
};

namespace SConst
{
	static const SMatrix3 ZeroSMatrix3 = SMatrix3 { 0.0f };
	static const SMatrix4 ZeroSMatrix4 = SMatrix4 { 0.0f };
	static const SMatrix3 IdentitySMatrix3 = SMatrix3 {
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f
	};
	static const SMatrix4 IdentitySMatrix4 = SMatrix4 {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
}

namespace SConvert
{
	inline SMatrix3 ToMatrix3(const SMatrix4& m)
	{
		return SMatrix3 {
			m.m[0], m.m[1], m.m[2],
			m.m[4], m.m[5], m.m[6],
			m.m[8], m.m[9], m.m[10],
		};
	}
}

#if USING_GLMATH
#include "glm/gtc/type_ptr.hpp"

namespace SConvert
{
	inline SMatrix3 ToMatrix3(const glm::mat3& m)
	{
		SMatrix3 result;
		std::memcpy(result.m, glm::value_ptr(m), sizeof(float) * 9);
		return result;
	}

	inline SMatrix4 ToMatrix4(const glm::mat4& m)
	{
		SMatrix4 result;
		std::memcpy(result.m, glm::value_ptr(m), sizeof(float) * 16);
		return result;
	}
}

#endif // USING_GLMATH

#if USING_EIGEN

namespace SConvert
{
	inline SMatrix3 ToMatrix3(const Eigen::Matrix3<float>& m)
	{
		SMatrix3 result;
		std::memcpy(result.m, m.data(), sizeof(float) * 9);
		return result;
	}

	inline SMatrix4 ToMatrix4(const Eigen::Matrix4<float>& m)
	{
		SMatrix4 result;
		std::memcpy(result.m, m.data(), sizeof(float) * 16);
		return result;
	}
}

#endif // USING_EIGEN
