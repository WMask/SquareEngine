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

inline SSize2F operator/(const SSize2F& v1, const SSize2F& v2)
{
	return SSize2F{ v1.width / v2.width, v1.height / v2.height };
}

inline SVector2 operator*(const SSize2F& size, const SVector2& v)
{
	return SVector2{ size.width * v.x, size.height * v.y };
}


/***************************************************************************
* Rect
*/

inline bool Contains(const SRectF& r, const SPoint2F& p)
{
	return p.x >= r.left && p.x < r.right && p.y >= r.top && p.y < r.bottom;
}

inline bool Contains(const SRect& r, const SPoint2& p)
{
	return p.x >= r.left && p.x < r.right && p.y >= r.top && p.y < r.bottom;
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

inline SMatrix4 operator*(const SMatrix4& a, const SMatrix4& b)
{
	return SConvert::ToMatrix4(DirectX::XMMatrixMultiply(SConvert::ToXMatrix(a), SConvert::ToXMatrix(b)));
}

namespace SMath
{
	inline SColor3F LerpColor3(const SColor3F& from, const SColor3F& to, float alpha)
	{
		DirectX::XMVECTOR xFrom = SConvert::ToXVector4(from);
		DirectX::XMVECTOR xTo = SConvert::ToXVector4(to);
		DirectX::XMVECTOR xVec = DirectX::XMVectorLerp(xFrom, xTo, alpha);
		return SConvert::ToColor3(xVec);
	}

	inline SColor4F LerpColor4(const SColor4F& from, const SColor4F& to, float alpha)
	{
		DirectX::XMVECTOR xFrom = SConvert::ToXVector4(from);
		DirectX::XMVECTOR xTo = SConvert::ToXVector4(to);
		DirectX::XMVECTOR xVec = DirectX::XMVectorLerp(xFrom, xTo, alpha);
		return SConvert::ToColor4(xVec);
	}

	inline SVector3 LerpVector3(const SVector3& from, const SVector3& to, float alpha)
	{
		DirectX::XMVECTOR xFrom = SConvert::ToXVector4(from);
		DirectX::XMVECTOR xTo = SConvert::ToXVector4(to);
		DirectX::XMVECTOR xVec = DirectX::XMVectorLerp(xFrom, xTo, alpha);
		return SConvert::ToVector3(xVec);
	}

	inline SVector3 Normalize(const SVector3& v)
	{
		return SConvert::ToVector3(DirectX::XMVector3Normalize(SConvert::ToXVector4(v)));
	}

	inline SVector4 Normalize(const SVector4& v)
	{
		return SConvert::ToVector4(DirectX::XMVector3Normalize(SConvert::ToXVector4(v)));
	}

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

	inline SMatrix4 OrthoMatrix(float widthPixels, float heightPixels, float nearPlane, float farPlane)
	{
		auto matrix = DirectX::XMMatrixOrthographicRH(widthPixels, heightPixels, 1.0f, -1.0f);
		return SConvert::ToMatrix4(matrix);
	}

	inline SMatrix4 OrthoMatrix(SSize2 viewportSize, float nearPlane, float farPlane)
	{
		return OrthoMatrix(static_cast<float>(viewportSize.width), static_cast<float>(viewportSize.height), nearPlane, farPlane);
	}

	inline SMatrix4 ProjectionMatrix(float fovDegrees, float aspectRatio, float nearPlane, float farPlane)
	{
		auto matrix = DirectX::XMMatrixPerspectiveFovRH(
			DirectX::XMConvertToRadians(fovDegrees),
			aspectRatio, nearPlane, farPlane
		);
		return SConvert::ToMatrix4(matrix);
	}

	inline SMatrix4 LookAtMatrix(const SVector3& from, const SVector3& to, bool bFlipY = false)
	{
		auto matrix = DirectX::XMMatrixLookAtRH(
			DirectX::XMVectorSet(from.x, from.y, from.z, 0.0f),
			DirectX::XMVectorSet(to.x, to.y, to.z, 0.0f),
			DirectX::XMVectorSet(0.0f, bFlipY ? -1.0f : 1.0f, 0.0f, 0.0f));
		return SConvert::ToMatrix4(matrix);
	}

	inline SMatrix4 TranslationMatrix(const SVector3& pos)
	{
		auto matrix = DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
		return SConvert::ToMatrix4(matrix);
	}

	inline SMatrix4 RotationMatrixY(float yRad)
	{
		auto matrix = DirectX::XMMatrixRotationY(yRad);
		return SConvert::ToMatrix4(matrix);
	}

	inline SMatrix4 ScaleMatrix(const SVector3& scale)
	{
		auto matrix = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
		return SConvert::ToMatrix4(matrix);
	}

	inline SMatrix4 ScaleMatrix2(const SVector2& scale)
	{
		auto matrix = DirectX::XMMatrixScaling(scale.x, scale.y, 1.0f);
		return SConvert::ToMatrix4(matrix);
	}

	inline SMatrix4 ScaleMatrix3(const SVector3& scale)
	{
		auto matrix = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
		return SConvert::ToMatrix4(matrix);
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
