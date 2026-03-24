/***************************************************************************
* SWorldInterface.h
*/

#pragma once

#include "Core/STypes.h"
#include "Core/SMathTypes.h"
#include "Core/SCamera.h"
#include "entt/entt.hpp"

#include <set>

#if defined(_MSC_VER)
# pragma warning(disable : 4251)
#endif


namespace SWorldInternal
{
	static const float SMinLayer = -0.9f;
	static const float SMaxLayer = 0.0f;
}

/** Ranged float (-0.9f, 0.0f) */
using SLayersRange = TRange<float, SWorldInternal::SMinLayer, SWorldInternal::SMaxLayer>;

/** 2D object layers. Z0 - front, Z9 - back. */
namespace SLayers
{
	static const SLayersRange Z0 = SLayersRange( 0.0f);
	static const SLayersRange Z1 = SLayersRange(-0.1f);
	static const SLayersRange Z2 = SLayersRange(-0.2f);
	static const SLayersRange Z3 = SLayersRange(-0.3f);
	static const SLayersRange Z4 = SLayersRange(-0.4f);
	static const SLayersRange Z5 = SLayersRange(-0.5f);
	static const SLayersRange Z6 = SLayersRange(-0.6f);
	static const SLayersRange Z7 = SLayersRange(-0.7f);
	static const SLayersRange Z8 = SLayersRange(-0.8f);
	static const SLayersRange Z9 = SLayersRange(-0.9f);
};


/***************************************************************************
* Game world scaling parameters for different resolutions.
* For resolutions not added to scaleList scale selected by resolution.y >= newScreenSize.y
*/
class SWorldScale
{
public:
	//
	entt::delegate<void(const SVector2&)> onScaleChanged;
	//
	struct ScalePair
	{
		ScalePair(SSize2 inResolution, SVector2 inScale) : resolution(inResolution), scale(inScale) {}
		inline bool operator<(const ScalePair& s) const { return resolution.height < s.resolution.height; }
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
		: scaleList{ {SSize2{ 1920, 1080 }
		, SConst::OneSVector2} }
		, scale(SConst::OneSVector2)
		, scaleFonts(true)
	{}
	//
	void UpdateWorldScale(SSize2 newScreenSize);
	//
	inline bool GetScaleFonts() const { return scaleFonts; }
	//
	inline const std::set<ScalePair>& GetScaleList() const { return scaleList; }
	//
	inline std::set<ScalePair>& GetScaleList() { return scaleList; }
	//
	inline SVector2 GetScale() const { return scale; }


protected:
	//
	std::set<ScalePair> scaleList;
	//
	SVector2 scale;
	//
	bool scaleFonts;
};


/***************************************************************************
* Game world interface */
class IWorld
{
public:
	/**
	* Subscribe to get changes of sprites global tint. */
	entt::delegate<void(const SColor3&)> onTintChanged;

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
	* Update world scale */
	virtual void UpdateWorldScale(SSize2 newScreenSize) = 0;
	/**
	* Get world scale */
	virtual const SWorldScale& GetWorldScale() const = 0;
	/**
	* Get world scale */
	virtual SWorldScale& GetWorldScale() = 0;
	/**
	* Get camera */
	virtual const class SCamera& GetCamera() const = 0;
	/**
	* Get camera */
	virtual class SCamera& GetCamera() = 0;
	/**
	* Set global sprites and widgets tint color (default White) */
	virtual void SetGlobalTint(const std::optional<SColor3>& tint) = 0;
	/**
	* Get global sprites and widgets tint color (default White) */
	virtual std::optional<SColor3> GetGlobalTint() const = 0;

};

using TWorldPtr = std::unique_ptr<IWorld>;
