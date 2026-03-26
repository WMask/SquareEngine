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

/** Textured sprite component */
struct STexturedSpriteComponent : public SColoredSpriteComponent
{
	SVector2 uvs[4];
	// id in texture manager
	STexID textureId;


public:
	//
	inline void SetDefaultUV()
	{
		uvs[0] = SVector2{ 1.0f,-1.0f};
		uvs[1] = SVector2{-1.0f,-1.0f};
		uvs[2] = SVector2{ 1.0f, 1.0f};
		uvs[3] = SVector2{-1.0f, 1.0f};
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
