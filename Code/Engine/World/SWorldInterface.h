/***************************************************************************
* SWorldInterface.h
*/

#pragma once

#include "World/SCamera.h"
#include "World/SFontInterface.h"
#include "Core/STypes.h"
#include "Core/SMathTypes.h"
#include "entt/entt.hpp"

#include <set>

#if defined(_MSC_VER)
# pragma warning(disable : 4251)
#endif


namespace SInternal
{
	static const float SMinLayer = -0.9f;
	static const float SMaxLayer = 0.0f;
}

/** Light resource id */
using SLightID = std::uint32_t;

/** Ranged float (-0.9f, 0.0f) */
using SLayersRange = TRange<float, SInternal::SMinLayer, SInternal::SMaxLayer>;

/** 2D object layers. Z0 - front, Z9 - back. */
namespace SLayers
{
	static const SLayersRange Z0 = SLayersRange(0.9f);
	static const SLayersRange Z1 = SLayersRange(0.8f);
	static const SLayersRange Z2 = SLayersRange(0.7f);
	static const SLayersRange Z3 = SLayersRange(0.6f);
	static const SLayersRange Z4 = SLayersRange(0.5f);
	static const SLayersRange Z5 = SLayersRange(0.4f);
	static const SLayersRange Z6 = SLayersRange(0.3f);
	static const SLayersRange Z7 = SLayersRange(0.2f);
	static const SLayersRange Z8 = SLayersRange(0.1f);
	static const SLayersRange Z9 = SLayersRange(0.0f);
};


/***************************************************************************
* Game world scaling parameters for different resolutions.
* For resolutions not added to scaleList scale selected by resolution.y >= newScreenSize.y
*/
class SWorldScale
{
public:
	//
	entt::delegate<void(SVector2)> onScaleChanged;
	//
	struct SScalePair
	{
		SScalePair(SSize2 inResolution, SVector2 inScale) : resolution(inResolution), scale(inScale) {}
		inline bool operator<(const SScalePair& s) const { return resolution.height < s.resolution.height; }
		SSize2 resolution;
		SVector2 scale;
	};
	//
	SWorldScale(const SWorldScale&) = default;
	//
	SWorldScale& operator=(const SWorldScale&) = default;


public:
	//
	SWorldScale()
		: scaleList{
			{ SSize2{ 3840, 2160 }, SVector2{ 1.0f / 0.5f, 1.0f / 0.5f } },
			{ SSize2{ 2560, 1440 }, SVector2{ 1.0f / 0.75f, 1.0f / 0.75f } },
			{ SSize2{ 1920, 1080 }, SVector2{ 1.0f, 1.0f } }, // Design x1 resolution is 1920x1080
			{ SSize2{ 1280, 720 }, SVector2{ 1.0f / 1.5f, 1.0f / 1.5f } },
			{ SSize2{ 800, 600 }, SVector2{ 1.0f / 1.8f, 1.0f / 1.8f } }
		}
		, scale(SConst::OneSVector2)
		, scaleFonts(true)
	{}
	//
	void UpdateWorldScale(SSize2 newScreenSize);
	//
	inline bool GetScaleFonts() const { return scaleFonts; }
	//
	inline const std::set<SScalePair>& GetScaleList() const { return scaleList; }
	//
	inline std::set<SScalePair>& GetScaleList() { return scaleList; }
	//
	inline SVector2 GetScale() const { return scale; }


protected:
	//
	std::set<SScalePair> scaleList;
	//
	SVector2 scale;
	//
	bool scaleFonts;
};


/***************************************************************************
* Game world interface */
class IWorld : public SUncopyable
{
public:
	/**
	* Subscribe to get changes of lights */
	entt::delegate<void(const IWorld&)> onLightsChanged;


public:
	/**
	* Destructor */
	virtual ~IWorld() {}
	/**
	* Get world entities */
	virtual entt::registry& GetEntities() = 0;
	/**
	* Get world entities */
	virtual const entt::registry& GetEntities() const = 0;
	/**
	* Remove all sprites and widgets */
	virtual void Clear(bool removeRooted = false) = 0;
	/**
	* Add directional light */
	virtual std::pair<SLightID, bool> AddDirectionalLight(const std::string_view& name, const SVector3& direction, const SColor3& color) = 0;
	/**
	* Add point light */
	virtual std::pair<SLightID, bool> AddPointLight(const std::string_view& name, const SVector3& pos, float distance, const SColor3& color) = 0;
	/**
	* Get lights data */
	virtual SLightsBuffer GetLightsData() const = 0;
	/**
	* Remove light */
	virtual bool RemoveLight(SLightID id) = 0;
	/**
	* Get lights count */
	virtual std::uint32_t GetNumLights(SLightID) const noexcept = 0;
	/**
	* Update world scale */
	virtual void UpdateWorldScale(SSize2 newScreenSize) = 0;
	/**
	* Get world scale */
	virtual const SWorldScale& GetScale() const = 0;
	/**
	* Get world scale */
	virtual SWorldScale& GetScale() = 0;
	/**
	* Get camera */
	virtual const SCamera& GetCamera() const = 0;
	/**
	* Get camera */
	virtual SCamera& GetCamera() = 0;
	/**
	* Get fonts */
	virtual const IFontSystem& GetFonts() const = 0;
	/**
	* Get fonts */
	virtual IFontSystem& GetFonts() = 0;

};

using TWorldPtr = std::unique_ptr<IWorld>;
