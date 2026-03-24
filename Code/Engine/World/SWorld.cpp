/***************************************************************************
* SWorld.cpp
*/

#include "World/SWorld.h"
#include "World/SWorldModule.h"


TWorldPtr CreateWorld(const SAppContext& context)
{
	return std::make_unique<SWorld>(context);
}

SWorld::SWorld(const SAppContext& inContext) : context(inContext)
{
	context.world = this;
}

SWorld::~SWorld()
{
	Clear(true);
}

void SWorld::Clear(bool removeRooted)
{
}

void SWorld::SetGlobalTint(const std::optional<SColor3>& tint)
{
	if (globalTint != tint)
	{
		globalTint = tint;
		onTintChanged(globalTint.value());
	}
}

void SWorld::UpdateWorldScale(SSize2 newScreenSize)
{
	worldScale.UpdateWorldScale(newScreenSize);
}

void SWorldScale::UpdateWorldScale(SSize2 newScreenSize)
{
	auto prevScale = scale;

	for (auto entry : scaleList)
	{
		// check equal
		if (entry.resolution == newScreenSize)
		{
			scale = entry.scale;
			if (prevScale != scale) onScaleChanged(scale);
			return;
		}
	}

	for (auto entry : scaleList)
	{
		// check greater
		if (entry.resolution.height >= newScreenSize.height)
		{
			scale = entry.scale;
			if (prevScale != scale) onScaleChanged(scale);
			return;
		}
	}

	if (scaleList.size() > 0)
	{
		// accept any
		scale = scaleList.begin()->scale;
		if (prevScale != scale) onScaleChanged(scale);
	}
}
