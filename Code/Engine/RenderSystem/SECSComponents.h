/***************************************************************************
* SECSComponents.h
*/

#pragma once

#include "RenderSystem/SRenderSystemInterface.h"


/** Colored sprite component */
struct SColoredSpriteComponent
{
	float rotation = 0.0f;
	//
	SVector3 position;
	//
	SSize2F size;
	//
	SColor4F colors[4];


public:
	//
	inline void SetWhiteColors()
	{
		colors[0] = colors[1] = colors[2] = colors[3] = SConst::OneSColor4F;
	}
	//
	inline void SetColors(const SColor4F& color)
	{
		colors[0] = colors[1] = colors[2] = colors[3] = color;
	}
	//
	inline void SetColors(const SColor3& color)
	{
		colors[0] = colors[1] = colors[2] = colors[3] = SConvert::FromSColor3(color);
	}
	//
	inline void SetColors(const SColor4F& lt, const SColor4F& rt, const SColor4F& rb, const SColor4F& lb)
	{
		colors[0] = rt;
		colors[1] = lt;
		colors[2] = rb;
		colors[3] = lb;
	}
	//
	inline void SetColors(const SColor3& lt, const SColor3& rt, const SColor3& rb, const SColor3& lb)
	{
		colors[0] = SConvert::FromSColor3(rt);
		colors[1] = SConvert::FromSColor3(lt);
		colors[2] = SConvert::FromSColor3(rb);
		colors[3] = SConvert::FromSColor3(lb);
	}
};


/** Texture component */
struct STexturedComponent
{
	// id in texture manager
	STexID texId;
};


/** Sprite UV component */
struct SSpriteUVComponent
{
	SVector2 uvs[4];


public:
	//
	inline void SetDefaultUV()
	{
		uvs[0] = SVector2{ 1.0f, 0.0f };
		uvs[1] = SVector2{ 0.0f, 0.0f };
		uvs[2] = SVector2{ 1.0f, 1.0f };
		uvs[3] = SVector2{ 0.0f, 1.0f };
	}
	//
	inline void SetUV(const SVector2& lt, const SVector2& rt, const SVector2& rb, const SVector2& lb)
	{
		uvs[0] = rt;
		uvs[1] = lt;
		uvs[2] = rb;
		uvs[3] = lb;
	}
};


/** Sprite frame animation component */
struct SSpriteFrameAnimComponent
{
	// start offset on frames sheet
	std::int32_t frameOffset;
	//
	std::int32_t framesCount;
	//
	std::int32_t framesPerSecond;
	// in pixels
	SSize2F frameSize;
	//
	float startTime;


public:
	//
	inline void SetAnim(std::int32_t inFrameOffset, std::int32_t inFramesCount,
		std::int32_t inFramesPerSecond, SSize2 inFrameSize, float inStartTime)
	{
		frameOffset = inFrameOffset;
		framesCount = inFramesCount;
		framesPerSecond = inFramesPerSecond;
		frameSize = SConvert::ToSize2F(inFrameSize);
		startTime = inStartTime;
	}
	//
	inline void GenerateFrameUV(float gameTime, SSize2 texSize, SSpriteUVComponent& outUV) const
	{
		const float frameTime = (1.0f / framesPerSecond);
		std::uint32_t frameId = frameOffset + (static_cast<std::uint32_t>((gameTime - startTime) / frameTime) % framesCount);
		if (frameId >= (frameOffset + framesCount)) frameId = frameOffset;

		const std::uint32_t columns = texSize.width / frameSize.width;
		if (columns > 0)
		{
			const std::uint32_t x = frameId % columns;
			const std::uint32_t y = frameId / columns;
			const float xs = frameSize.width / static_cast<float>(texSize.width);
			const float ys = frameSize.height / static_cast<float>(texSize.height);
			const float xf = static_cast<float>(x * frameSize.width) / static_cast<float>(texSize.width);
			const float yf = static_cast<float>(y * frameSize.height) / static_cast<float>(texSize.height);

			outUV.uvs[0] = SVector2{ xf + xs, yf }; // rt
			outUV.uvs[1] = SVector2{ xf,      yf }; // lt
			outUV.uvs[2] = SVector2{ xf + xs, yf + ys }; // rb
			outUV.uvs[3] = SVector2{ xf,      yf + ys }; // lb
		}
	}
};

/** Widget component */
struct SWidgetComponent
{
	bool bHovered{};
};
