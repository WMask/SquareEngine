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
		colors[0] = lt;
		colors[1] = rt;
		colors[2] = rb;
		colors[3] = lb;
	}
	//
	inline void SetColors(const SColor3& lt, const SColor3& rt, const SColor3& rb, const SColor3& lb)
	{
		colors[0] = SConvert::FromSColor3(lt);
		colors[1] = SConvert::FromSColor3(rt);
		colors[2] = SConvert::FromSColor3(rb);
		colors[3] = SConvert::FromSColor3(lb);
	}
};
