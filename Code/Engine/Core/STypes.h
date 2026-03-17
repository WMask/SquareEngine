/***************************************************************************
* STypes.h
*/

#pragma once

#include <string>
#include <optional>


/** Render system type */
enum class SRSType
{
	DX11,
	DX12,
	Vulkan,
	Metal
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

inline SColor4F FromSColor3(const SColor3& color)
{
	return SColor4F {
		static_cast<float>(color.r) / 255.0f,
		static_cast<float>(color.g) / 255.0f,
		static_cast<float>(color.b) / 255.0f,
		1.0f
	};
}
