/***************************************************************************
* SCamera.h
*/

#pragma once

#include "Core/SMath.h"
#include "entt/entt.hpp"


/** Camera space */
enum class SCameraSpace
{
	Camera2D,
	Camera3D
};

/***************************************************************************
* Game camera
*/
class SCamera
{
public:
	//
	entt::delegate<void(const SCamera&)> onViewChanged;

public:
	/**
	* Constructor */
	SCamera()
		: position2d(SConst::ZeroSVector3)
		, target2d(SConst::ZeroSVector3)
		, position3d(SConst::ZeroSVector3)
		, target3d(SConst::ZeroSVector3)
		, fovDegrees(90.0f)
	{}
	/**
	* Set camera */
	inline void Set(SCameraSpace space, SVector3 newPos, SVector3 newTarget)
	{
		if (space == SCameraSpace::Camera2D)
		{
			position2d = newPos;
			target2d = newTarget;
		}
		else
		{
			position3d = newPos;
			target3d = newTarget;
		}

		if (onViewChanged) onViewChanged(*this);
	}
	/**
	* Set camera */
	inline void Set(SVector3 newPos, SVector3 newTarget, float inFovDegrees)
	{
		position3d = newPos;
		target3d = newTarget;
		fovDegrees = inFovDegrees;

		if (onViewChanged) onViewChanged(*this);
	}
	/**
	* Move camera */
	inline void Move(SCameraSpace space, SVector3 newPosOffset, SVector3 newTargetOffset)
	{
		if (space == SCameraSpace::Camera2D)
		{
			position2d = position2d + newPosOffset;
			target2d = target2d + newTargetOffset;
		}
		else
		{
			position3d = position3d + newPosOffset;
			target3d = target3d + newTargetOffset;
		}

		if (onViewChanged) onViewChanged(*this);
	}
	/**
	* Get position */
	inline SVector3 GetPosition(SCameraSpace space) const noexcept
	{
		return (space == SCameraSpace::Camera2D) ? position2d : position3d;
	}
	/**
	* Get target */
	inline SVector3 GetTarget(SCameraSpace space) const noexcept
	{
		return (space == SCameraSpace::Camera2D) ? target2d : target3d;
	}
	/**
	* Get FOV in degrees */
	inline float GetFOV() const noexcept
	{
		return fovDegrees;
	}


protected:
	//
	SVector3 position2d;
	//
	SVector3 target2d;
	//
	SVector3 position3d;
	//
	SVector3 target3d;
	//
	float fovDegrees;

};
