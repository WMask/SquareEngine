/***************************************************************************
* SCamera.h
*/

#pragma once

#include "Core/SMath.h"


/***************************************************************************
* Game camera
*/
class SCamera
{
public:
	/**
	* Constructor */
	SCamera() : position(SConst::ZeroSVector3), target(SConst::ZeroSVector3) {}
	/**
	* Set camera */
	inline void Set(SVector3 newPos, SVector3 newTarget)
	{
		position = newPos;
		target = newTarget;
	}
	/**
	* Move camera */
	inline void Move(SVector3 newPosOffset, SVector3 newTargetOffset)
	{
		position = position + newPosOffset;
		target = target + newTargetOffset;
	}
	/**
	* Get position */
	inline SVector3 GetPosition() const noexcept { return position; }
	/**
	* Get target */
	inline SVector3 GetTarget() const noexcept { return target; }


protected:
	//
	SVector3 position;
	//
	SVector3 target;

};
