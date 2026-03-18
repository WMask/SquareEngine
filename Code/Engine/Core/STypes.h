/***************************************************************************
* STypes.h
*/

#pragma once

#include <string>
#include <optional>
#include <algorithm>


/** Render system type */
enum class SRSType
{
	DX11,
	DX12,
	Vulkan,
	Metal
};

/** Clamp float value */
inline float SClamp(float value, float minValue, float maxValue)
{
	return std::clamp(value, minValue, maxValue);
}

/** Clamp int value */
inline std::int32_t SClamp(std::int32_t value, std::int32_t minValue, std::int32_t maxValue)
{
	return std::clamp(value, minValue, maxValue);
}

/** Range struct */
template<typename T, const T& minValue, const T& maxValue>
struct SRange
{
	SRange() : value(static_cast<T>(0)) {}
	//
	SRange(const SRange& ref) : value(ref.value) {}
	//
	SRange(T inValue) : value(SClamp(inValue, minValue, maxValue)) {}
	//
	inline SRange& operator = (const SRange& ref) { value = ref.value; return *this; }
	//
	inline operator T () const { return value; }
	//
	inline T* get() const { return const_cast<T*>(&value); }
	//
	T value;
};


/***************************************************************************
* Color
*/

// Color
struct SColor3
{
	std::uint8_t r;
	std::uint8_t g;
	std::uint8_t b;

	bool operator==(const SColor3&) const = default;
};

// Color
struct SColor3F
{
	float r;
	float g;
	float b;

	bool operator==(const SColor3F&) const = default;
};

// Color with alpha
struct SColor4F : public SColor3F
{
	float a;

	bool operator==(const SColor4F&) const = default;
	inline operator const float*() const { return &r; }
};

namespace SConst
{
	static const SColor3 OneSColor3 = SColor3{ 255, 255, 255 };
	static const SColor3F OneSColor3F = SColor3F{ 1.0f, 1.0f, 1.0f };
	static const SColor4F OneSColor4F = SColor4F{ 1.0f, 1.0f, 1.0f, 1.0f };
}

namespace SConvert
{
	inline SColor4F FromSColor3(const SColor3& color)
	{
		return SColor4F{
			static_cast<float>(color.r) / 255.0f,
			static_cast<float>(color.g) / 255.0f,
			static_cast<float>(color.b) / 255.0f,
			1.0f
		};
	}
}