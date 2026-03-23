/***************************************************************************
* SMath.h
*/

#pragma once

#include "Core/CoreModule.h"
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

inline SVector2 operator*(const SSize2F& size, const SVector2& v)
{
	return SVector2{ size.width * v.x, size.height * v.y };
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

#if USING_DXMATH

namespace SMath
{
	inline SMatrix3 InverseM3(const SMatrix3& m)
	{
		DirectX::XMVECTOR determinant;
		DirectX::XMFLOAT3X3 tmp;
		memcpy(&tmp, m.m, sizeof(float) * 9);
		DirectX::XMMATRIX matrix(XMLoadFloat3x3(&tmp));
		return SConvert::ToMatrix3(DirectX::XMMatrixInverse(&determinant, matrix));
	}

	inline SMatrix4 InverseM4(const SMatrix4& m)
	{
		DirectX::XMVECTOR determinant;
		DirectX::XMFLOAT4X4 tmp;
		memcpy(&tmp, m.m, sizeof(float) * 16);
		DirectX::XMMATRIX matrix(XMLoadFloat4x4(&tmp));
		return SConvert::ToMatrix4(DirectX::XMMatrixInverse(&determinant, matrix));
	}

	inline SMatrix3 TransposeM3(const SMatrix3& m)
	{
		DirectX::XMFLOAT3X3 tmp;
		memcpy(&tmp, m.m, sizeof(float) * 9);
		DirectX::XMMATRIX matrix(XMLoadFloat3x3(&tmp));
		return SConvert::ToMatrix3(DirectX::XMMatrixTranspose(matrix));
	}

	inline SMatrix4 TransposeM4(const SMatrix4& m)
	{
		DirectX::XMFLOAT4X4 tmp;
		memcpy(&tmp, m.m, sizeof(float) * 16);
		DirectX::XMMATRIX matrix(XMLoadFloat4x4(&tmp));
		return SConvert::ToMatrix4(DirectX::XMMatrixTranspose(matrix));
	}

	inline SMatrix4 OrthoMatrix(float widthPixels, float heightPixels, float nearPlane, float farPlane, bool flipY = true)
	{
		auto matrix = DirectX::XMMatrixOrthographicLH(widthPixels, heightPixels, nearPlane, farPlane);
		if (flipY) matrix = DirectX::XMMatrixMultiply(matrix, DirectX::XMMatrixScaling(1.0f, -1.0f, 1.0f));
		return SConvert::ToMatrix4(DirectX::XMMatrixTranspose(matrix));
	}

	inline SMatrix4 OrthoMatrix(SSize2 viewportSize, float nearPlane, float farPlane, bool flipY = true)
	{
		return OrthoMatrix(static_cast<float>(viewportSize.width), static_cast<float>(viewportSize.height), nearPlane, farPlane, flipY);
	}

	inline SMatrix4 LookAtMatrix(const SVector3& from, const SVector3& to)
	{
		auto matrix = DirectX::XMMatrixLookAtLH(
			DirectX::XMVectorSet(from.x, from.y, from.z, 0.0f),
			DirectX::XMVectorSet(to.x, to.y, to.z, 0.0f),
			DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
		return SConvert::ToMatrix4(DirectX::XMMatrixTranspose(matrix));
	}

	inline SMatrix4 TranslationMatrix(const SVector3& pos)
	{
		return SConvert::ToMatrix4(DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z));
	}

	inline SMatrix4 TransformMatrix(const SVector3& pos, const SVector2& scale, float rotZ = 0.0f, bool flipY = true)
	{
		auto matrix = DirectX::XMMatrixTransformation(
			SConst::ZeroXVec4, SConst::ZeroXVec4,
			DirectX::XMVectorSet(scale.x, flipY ? -scale.y : scale.y, 0.0f, 1.0f),
			SConst::ZeroXVec4,
			DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f), rotZ),
			DirectX::XMVectorSet(pos.x, pos.y, pos.z, 1.0f));
		return SConvert::ToMatrix4(DirectX::XMMatrixTranspose(matrix));
	}
}

#endif // USING_DXMATH
