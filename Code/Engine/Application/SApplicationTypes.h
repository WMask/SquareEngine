/***************************************************************************
* SApplicationTypes.h
*/

#pragma once

#include "Core/STypes.h"
#include <unordered_map>
#include <any>


/** Key state: Up, Down */
enum class SKeyState
{
	Up,
	Down
};

/** Window mode */
enum class SAppMode { Windowed, Fullscreen };

/** Application features */
enum class SAppFeature {
	/**
	* [bool] Set vertical synchronization mode */
	VSync,
	/**
	* [bool] Set No Delay mode.
	* If true: high FPS and update rate.
	* If true and VSync false: processor core utilization 100%, highest FPS and update rate.  */
	NoDelay,
	/**
	* [bool] Allow high frequency timer. Increase frame rate if NoDelay is false.
	* On Windows changes this value for whole OS that may increase power usage. */
	HighFrequencyTimer,
	/**
	* [bool] Set fullscreen mode type.
	* If true: actually changes monitor resolution. If VSync true: FPS - equal to monitor highest refresh rate.
	* If false: fake windowed fullscreen mode. If VSync true: FPS - default OS refresh rate. */
	AllowFullscreen,
	/**
	* [SColor3] Render target clear color. Default is blue. Set to empty std::any to skip clear render target color. */
	ClearScreenColor,
	/**
	* [int] Tasks per thread. Default: SConst::DefaultTasksPerThread. */
	ThreadPoolTasksPerThread,
	/**
	* [int] Threads count. Default: SConst::DefaultThreadsInPool. */
	ThreadPoolThreadsCount,
	/**
	* [bool] Enable thread pool tasks logs. */
	ThreadPoolDebugTrace,
	/**
	* [bool] Enable render system logs. */
	RenderSystemDebugTrace
};

using SAppFeaturesMap = std::unordered_map<SAppFeature, std::any>;

/** Get boolean feature value */
inline bool GetFeatureFlag(const SAppFeaturesMap& features, SAppFeature type)
{
	auto featureIt = features.find(type);
	return (featureIt != features.end() && featureIt->second.has_value() && std::any_cast<bool>(featureIt->second));
}

/** Get int feature value */
inline std::int32_t GetFeatureValue(const SAppFeaturesMap& features, SAppFeature type)
{
	auto featureIt = features.find(type);
	return (featureIt != features.end() && featureIt->second.has_value()) ? std::any_cast<std::int32_t>(featureIt->second) : -1;
}

/** Get color feature value */
inline std::pair<SColor3, bool> GetFeatureColor(const SAppFeaturesMap& features, SAppFeature type)
{
	auto colorIt = features.find(SAppFeature::ClearScreenColor);
	if (colorIt->second.has_value())
	{
		return { std::any_cast<SColor3>(colorIt->second), true };
	}

	return { SConst::OneSColor3, false };
}

/** Application context */
struct SAppContext
{
	class IApplication* app{};
	//
	class IInputSystem* input{};
	//
	class IThreadPool* pool{};
	//
	class IWorld* world{};
	//
	class IRenderSystem* render{};
	//
	float deltaSeconds{};
	//
	float gameTime{};
	//
	std::uint32_t gameFrame{};
	//
	std::uint32_t fps{};
};
