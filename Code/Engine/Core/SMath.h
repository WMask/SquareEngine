/***************************************************************************
* SMath.h
*/

#pragma once

#include "Core/SCoreModule.h"
#include "Core/SMathTypes.h"


/***************************************************************************
* Vectors
*/

inline SVector2 operator+(const SVector2& v1, const SVector2& v2)
{
	return SVector2{ v1.x + v2.x, v1.y + v2.y };
}

inline SVector2 operator-(const SVector2& v1, const SVector2& v2)
{
	return SVector2{ v1.x - v2.x, v1.y - v2.y };
}

inline SVector2 operator*(const SVector2& v1, const SVector2& v2)
{
	return SVector2{ v1.x * v2.x, v1.y * v2.y };
}

inline SVector2 operator+(const SVector2& v1, float v2)
{
	return SVector2{ v1.x + v2, v1.y + v2 };
}

inline SVector2 operator-(const SVector2& v1, float v2)
{
	return SVector2{ v1.x - v2, v1.y - v2 };
}

inline SVector2 operator*(const SVector2& v1, float v2)
{
	return SVector2{ v1.x * v2, v1.y * v2 };
}

inline SVector2 operator/(const SVector2& v1, float v2)
{
	return SVector2{ v1.x / v2, v1.y / v2 };
}

inline SVector3 operator+(const SVector3& v1, const SVector3& v2)
{
	return SVector3{ v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
}

inline SVector3 operator-(const SVector3& v1, const SVector3& v2)
{
	return SVector3{ v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
}

inline SVector3 operator*(const SVector3& v1, const SVector3& v2)
{
	return SVector3{ v1.x * v2.x, v1.y * v2.y, v1.z * v2.z };
}

inline SVector3 operator+(const SVector3& v1, float v2)
{
	return SVector3{ v1.x + v2, v1.y + v2, v1.z + v2 };
}

inline SVector3 operator-(const SVector3& v1, float v2)
{
	return SVector3{ v1.x - v2, v1.y - v2, v1.z - v2 };
}

inline SVector3 operator*(const SVector3& v1, float v2)
{
	return SVector3{ v1.x * v2, v1.y * v2, v1.z * v2 };
}

inline SVector3 operator/(const SVector3& v1, float v2)
{
	return SVector3{ v1.x / v2, v1.y / v2, v1.z / v2 };
}

inline SVector4 operator+(const SVector4& v1, const SVector4& v2)
{
	return SVector4{ v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w };
}

inline SVector4 operator-(const SVector4& v1, const SVector4& v2)
{
	return SVector4{ v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w };
}

inline SVector4 operator*(const SVector4& v1, const SVector4& v2)
{
	return SVector4{ v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w };
}

inline SVector4 operator+(const SVector4& v1, float v2)
{
	return SVector4{ v1.x + v2, v1.y + v2, v1.z + v2, v1.w + v2 };
}

inline SVector4 operator-(const SVector4& v1, float v2)
{
	return SVector4{ v1.x - v2, v1.y - v2, v1.z - v2, v1.w - v2 };
}

inline SVector4 operator*(const SVector4& v1, float v2)
{
	return SVector4{ v1.x * v2, v1.y * v2, v1.z * v2, v1.w * v2 };
}

inline SVector4 operator/(const SVector4& v1, float v2)
{
	return SVector4{ v1.x / v2, v1.y / v2, v1.z / v2, v1.w / v2 };
}


/***************************************************************************
* Point & Size
*/

inline SPoint2 operator+(const SPoint2& v1, const SPoint2& v2)
{
	return SPoint2{ v1.x + v2.x, v1.y + v2.y };
}

inline SPoint2 operator-(const SPoint2& v1, const SPoint2& v2)
{
	return SPoint2{ v1.x - v2.x, v1.y - v2.y };
}

inline SPoint2 operator*(const SPoint2& v1, const SPoint2& v2)
{
	return SPoint2{ v1.x * v2.x, v1.y * v2.y };
}

inline SPoint2 operator+(const SPoint2& v1, std::int32_t v2)
{
	return SPoint2{ v1.x + v2, v1.y + v2 };
}

inline SPoint2 operator-(const SPoint2& v1, std::int32_t v2)
{
	return SPoint2{ v1.x - v2, v1.y - v2 };
}

inline SPoint2 operator*(const SPoint2& v1, std::int32_t v2)
{
	return SPoint2{ v1.x * v2, v1.y * v2 };
}

inline SPoint2 operator/(const SPoint2& v1, std::int32_t v2)
{
	return SPoint2{ v1.x / v2, v1.y / v2 };
}

inline SPoint3 operator+(const SPoint3& v1, const SPoint3& v2)
{
	return SPoint3{ v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
}

inline SPoint3 operator-(const SPoint3& v1, const SPoint3& v2)
{
	return SPoint3{ v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
}

inline SPoint3 operator*(const SPoint3& v1, const SPoint3& v2)
{
	return SPoint3{ v1.x * v2.x, v1.y * v2.y, v1.z * v2.z };
}

inline SPoint3 operator+(const SPoint3& v1, std::int32_t v2)
{
	return SPoint3{ v1.x + v2, v1.y + v2, v1.z + v2 };
}

inline SPoint3 operator-(const SPoint3& v1, std::int32_t v2)
{
	return SPoint3{ v1.x - v2, v1.y - v2, v1.z - v2 };
}

inline SPoint3 operator*(const SPoint3& v1, std::int32_t v2)
{
	return SPoint3{ v1.x * v2, v1.y * v2, v1.z * v2 };
}

inline SPoint3 operator/(const SPoint3& v1, std::int32_t v2)
{
	return SPoint3{ v1.x / v2, v1.y / v2, v1.z / v2 };
}

inline SSize2 operator+(const SSize2& v1, std::int32_t v2)
{
	return SSize2{ v1.width + v2, v1.height + v2 };
}

inline SSize2 operator-(const SSize2& v1, std::int32_t v2)
{
	return SSize2{ v1.width - v2, v1.height - v2 };
}

inline SSize2 operator*(const SSize2& v1, std::int32_t v2)
{
	return SSize2{ v1.width * v2, v1.height * v2 };
}

inline SSize2 operator/(const SSize2& v1, std::int32_t v2)
{
	return SSize2{ v1.width / v2, v1.height / v2 };
}


/***************************************************************************
* Matrix
*/

#if USING_GLMATH

namespace SMath
{
	inline SMatrix3 InverseM3(const SMatrix3& m)
	{
		glm::mat3 result = glm::make_mat3(m.m);
		return SConvert::ToMatrix3(glm::inverse(result));
	}

	inline SMatrix4 InverseM4(const SMatrix4& m)
	{
		glm::mat4 result = glm::make_mat4(m.m);
		return SConvert::ToMatrix4(glm::inverse(result));
	}

	inline SMatrix3 TransposeM3(const SMatrix3& m)
	{
		glm::mat3 result = glm::make_mat3(m.m);
		return SConvert::ToMatrix3(glm::transpose(result));
	}

	inline SMatrix4 TransposeM4(const SMatrix4& m)
	{
		glm::mat4 result = glm::make_mat4(m.m);
		return SConvert::ToMatrix4(glm::transpose(result));
	}
}

#endif // USING_GLMATH

#if USING_EIGEN

namespace SMath
{
	inline SMatrix3 InverseM3(const SMatrix3& m)
	{
		Eigen::Matrix3<float> result(m.m);
		return SConvert::ToMatrix3(result.inverse());
	}

	inline SMatrix4 InverseM4(const SMatrix4& m)
	{
		Eigen::Matrix4<float> result(m.m);
		return SConvert::ToMatrix4(result.inverse());
	}

	inline SMatrix3 TransposeM3(const SMatrix3& m)
	{
		Eigen::Matrix3<float> result(m.m);
		return SConvert::ToMatrix3(result.transpose());
	}

	inline SMatrix4 TransposeM4(const SMatrix4& m)
	{
		Eigen::Matrix4<float> result(m.m);
		return SConvert::ToMatrix4(result.transpose());
	}
}

#endif // USING_EIGEN
