/***************************************************************************
* SMathTypes.h
*/

#pragma once

#include "SConfig.h"
#include <cstdint>
#include <cstring>


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
	inline operator const float*() const { return &x; }
};

// Quaternion rotation
using SQuat = SVector4;

// Object 3D transform
struct STransform
{
	SVector3 position;
	SQuat rotation;
	SVector3 scale;
};

namespace SConst
{
	constexpr SVector2 ZeroSVector2 = SVector2{};
	constexpr SVector3 ZeroSVector3 = SVector3{};
	constexpr SVector4 ZeroSVector4 = SVector4{};
	constexpr SVector4 OneSVector4 = SVector4{ 1.0f, 1.0f, 1.0f, 1.0f };
	constexpr SVector3 OneSVector3 = SVector3{ 1.0f, 1.0f, 1.0f };
	constexpr SVector2 OneSVector2 = SVector2{ 1.0f, 1.0f };
}

namespace SConvert
{
	inline SVector4 ToVector4(const SVector2& v, bool bOneW = true)
	{
		return SVector4{ v.x, v.y, 0.0f, bOneW ? 1.0f : 0.0f };
	}

	inline SVector4 ToVector4(const SVector3& v, bool bOneW = true)
	{
		return SVector4{ v.x, v.y, v.z, bOneW ? 1.0f : 0.0f };
	}
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

#if USING_DXMATH
#include <directxmath.h>

namespace SConvert
{
	inline SVector2 ToVector2(DirectX::XMVECTOR v)
	{
		return SVector2{ DirectX::XMVectorGetX(v), DirectX::XMVectorGetY(v) };
	}

	inline SVector3 ToVector3(DirectX::XMVECTOR v)
	{
		return SVector3{ DirectX::XMVectorGetX(v), DirectX::XMVectorGetY(v), DirectX::XMVectorGetZ(v) };
	}

	inline SVector4 ToVector4(DirectX::XMVECTOR v)
	{
		return SVector4{ DirectX::XMVectorGetX(v), DirectX::XMVectorGetY(v), DirectX::XMVectorGetZ(v), DirectX::XMVectorGetW(v) };
	}

	inline DirectX::XMVECTOR ToXVector4(const SVector3& v)
	{
		return DirectX::XMVectorSet(v.x, v.y, v.z, 1.0f);
	}

	inline DirectX::XMVECTOR ToXVector4(const SVector4& v)
	{
		return DirectX::XMVectorSet(v.x, v.y, v.z, v.w);
	}

	inline SQuat ToQuat(float pitchDegrees, float yawDegrees, float rollDegrees)
	{
		return ToVector4(DirectX::XMQuaternionRotationRollPitchYaw(
			DirectX::XMConvertToRadians(pitchDegrees),
			DirectX::XMConvertToRadians(yawDegrees),
			DirectX::XMConvertToRadians(rollDegrees))
		);
	}
}

namespace SConst
{
	static const DirectX::XMVECTOR ZeroXVec4 = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	static const DirectX::XMVECTOR OneXVec4 = DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
}

#endif // USING_DXMATH


/***************************************************************************
* Point & Size & Rect
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

using SPoint2F = SVector2;

// Size 2D
struct SSize2
{
	std::uint32_t width;
	std::uint32_t height;

	bool operator==(const SSize2&) const = default;
	inline bool IsZero() const noexcept { return (width == 0u) && (height == 0u); }
};

// Size 2D
struct SSize2F
{
	float width;
	float height;

	bool operator==(const SSize2F&) const = default;
};

// Size 3D
struct SSize3
{
	std::uint32_t x;
	std::uint32_t y;
	std::uint32_t z;

	bool operator==(const SSize3&) const = default;
};

using SSize3F = SVector3;

// Rect
struct SRectF
{
	float left;
	float top;
	float right;
	float bottom;

	bool operator==(const SRectF&) const = default;
};

// Rect
struct SRect
{
	std::int32_t left;
	std::int32_t top;
	std::int32_t right;
	std::int32_t bottom;

	bool operator==(const SRect&) const = default;
};

namespace SConst
{
	constexpr SPoint2 ZeroSPoint2 = SPoint2{};
	constexpr SPoint3 ZeroSPoint3 = SPoint3{};
	constexpr SSize2 ZeroSSize2 = SSize2{};
	constexpr SSize3 ZeroSSize3 = SSize3{};
}

namespace SConvert
{
	inline SVector2 ToVector2(const SSize2& size)
	{
		return SVector2{ static_cast<float>(size.width), static_cast<float>(size.height) };
	}

	inline SVector2 ToVector2(const SSize2F& size)
	{
		return SVector2{ size.width, size.height };
	}

	inline SVector2 ToVector2(const SPoint2& p)
	{
		return SVector2{ static_cast<float>(p.x), static_cast<float>(p.y) };
	}

	inline SSize2F ToSize2F(const SSize2& size)
	{
		return SSize2F{ static_cast<float>(size.width), static_cast<float>(size.height) };
	}

	inline SPoint2 ToPoint2(const SPoint2F& p)
	{
		return SPoint2{ static_cast<std::int32_t>(p.x), static_cast<std::int32_t>(p.y) };
	}

	inline SPoint2F ToPoint2(const SPoint2& p)
	{
		return SPoint2F{ static_cast<float>(p.x), static_cast<float>(p.y) };
	}

	inline SSize2 ToSize2(const SSize2F& size)
	{
		return SSize2{ static_cast<std::uint32_t>(size.width), static_cast<std::uint32_t>(size.height) };
	}

	inline SRect ToRect(const SPoint2& pos, const SSize2& size)
	{
		const std::int32_t hw = static_cast<std::int32_t>(size.width) / 2;
		const std::int32_t hh = static_cast<std::int32_t>(size.height) / 2;
		return SRect{ pos.x - hw, pos.y - hh, pos.x + hw, pos.y + hh };
	}

	inline SRect ToRect(const SPoint2F& pos, const SSize2F& size)
	{
		const std::int32_t hw = static_cast<std::int32_t>(size.width) / 2;
		const std::int32_t hh = static_cast<std::int32_t>(size.height) / 2;
		return SRect {
			static_cast<std::int32_t>(pos.x) - hw,
			static_cast<std::int32_t>(pos.y) - hh,
			static_cast<std::int32_t>(pos.x) + hw,
			static_cast<std::int32_t>(pos.y) + hh
		};
	}

	inline SRectF ToRectF(const SPoint2F& pos, const SSize2F& size)
	{
		const float hw = size.width / 2.0f;
		const float hh = size.height / 2.0f;
		return SRectF{ pos.x - hw, pos.y - hh, pos.x + hw, pos.y + hh };
	}
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
	constexpr SMatrix3 ZeroSMatrix3 = SMatrix3 { 0.0f };
	constexpr SMatrix4 ZeroSMatrix4 = SMatrix4 { 0.0f };
	constexpr SMatrix3 IdentitySMatrix3 = SMatrix3 {
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f
	};
	constexpr SMatrix4 IdentitySMatrix4 = SMatrix4 {
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

#if USING_DXMATH

namespace SConvert
{
	inline SMatrix3 ToMatrix3(const DirectX::XMMATRIX& m)
	{
		SMatrix3 result;
		DirectX::XMFLOAT3X3 m3;
		XMStoreFloat3x3(&m3, m);
		std::memcpy(&result, &m3._11, sizeof(float) * 9);
		return result;
	}

	inline SMatrix4 ToMatrix4(const DirectX::XMMATRIX& m)
	{
		SMatrix4 result;
		DirectX::XMFLOAT4X4 m4;
		XMStoreFloat4x4(&m4, m);
		std::memcpy(&result,  &m4._11, sizeof(float) * 16);
		return result;
	}

	inline DirectX::XMMATRIX ToXMatrix(const SMatrix4& m)
	{
		DirectX::XMFLOAT4X4 m4;
		std::memcpy(&m4._11 , m.m, sizeof(float) * 16);
		return DirectX::XMLoadFloat4x4(&m4);
	}
}

#endif // USING_DXMATH
