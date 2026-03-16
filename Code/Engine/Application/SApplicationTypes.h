/***************************************************************************
* SApplicationTypes.h
*/

#pragma once

#include "Core/STypes.h"
#include <unordered_map>


/** Window mode */
enum class SAppMode { Windowed, Fullscreen };

/** Application features */
enum class SAppFeature {
	/**
	* [bool] Set vertical synchronization mode */
	VSync,
	/**
	* [bool] Set fullscreen mode type.
	* If true: actually changes monitor resolution. If VSync true: FPS - equal to monitor highest refresh rate.
	* If false: fake windowed fullscreen mode. If VSync true: FPS - default OS refresh rate. */
	AllowFullscreen,
	/**
	* [bool] Set No Delay mode.
	* If true: high FPS and update rate.
	* If true and VSync false: processor core utilization 100%, highest FPS and update rate.  */
	NoDelay
};

using SAppFeaturesMap = std::unordered_map<SAppFeature, SAny>;

/** Application context */
struct SAppContext
{
	class IApplication* app{};
	//
	void* windowHandle{};
	//
	float gameTime{};
};
