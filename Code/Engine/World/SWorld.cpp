/***************************************************************************
* SWorld.cpp
*/

#include "World/SWorld.h"
#include "World/SWorldModule.h"


namespace SConst
{
	static const float DirectionLightType = 0.1f;
	static const float PointLightType = 0.9f;
}

TWorldPtr CreateWorld(const SAppContext& context)
{
	return std::make_unique<SWorld>(context);
}

SWorld::SWorld(const SAppContext& inContext) : context(&inContext), fonts(&inContext)
{
}

SWorld::~SWorld()
{
	Clear(true);
}

void SWorld::Clear(bool removeRooted)
{
	lights.clear();
	if (onLightsChanged) onLightsChanged(*this);
}

std::pair<SLightID, bool> SWorld::AddDirectionalLight(const std::string_view& name, const SVector3& direction, const SColor3& color)
{
	if (lights.size() < SConst::MaxLightsCount)
	{
		SLightID id = ResourceID<SLightID>(name);
		SLight light {
			SVector4 { direction.x, direction.y, direction.z, SConst::DirectionLightType },
			SVector4 {
				static_cast<float>(color.r) / 255.0f,
				static_cast<float>(color.g) / 255.0f,
				static_cast<float>(color.b) / 255.0f,
				0.0f
			}
		};
		lights.emplace(id, light);
		if (onLightsChanged) onLightsChanged(*this);

		return { id, true };
	}

	return { 0u, false };
}

std::pair<SLightID, bool> SWorld::AddPointLight(const std::string_view& name, const SVector3& pos, float distance, const SColor3& color)
{
	if (lights.size() < SConst::MaxLightsCount)
	{
		SLightID id = ResourceID<SLightID>(name);
		SLight light {
			SVector4 { pos.x, pos.y, pos.z, SConst::PointLightType },
			SVector4 {
				static_cast<float>(color.r) / 255.0f,
				static_cast<float>(color.g) / 255.0f,
				static_cast<float>(color.b) / 255.0f,
				distance
			}
		};
		lights.emplace(id, light);
		if (onLightsChanged) onLightsChanged(*this);

		return { id, true };
	}

	return { 0u, false };
}

bool SWorld::RemoveLight(SLightID id)
{
	if (lights.contains(id))
	{
		lights.erase(id);
		if (onLightsChanged) onLightsChanged(*this);
		return true;
	}

	return false;
}

SLightsBuffer SWorld::GetLightsData() const
{
	SLightsBuffer data;
	data.numLights = lights.size();

	if (data.numLights > 0 && data.numLights < SConst::MaxLightsCount)
	{
		std::uint32_t lightId = 0;

		for (auto& light : lights)
		{
			data.lightVec[lightId] = light.second.lightVec;
			data.lightColor[lightId] = light.second.lightColor;
			lightId++;
		}
	}
	else
	{
		data.numLights = 0u;
	}

	return data;
}

void SWorld::SetGlobalTint(const std::optional<SColor3>& tint)
{
	if (globalTint != tint)
	{
		globalTint = tint;
		onGlobalTintChanged(globalTint.value());
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
			if (prevScale != scale && onScaleChanged) onScaleChanged(scale);
			return;
		}
	}

	for (auto entry : scaleList)
	{
		// check greater
		if (entry.resolution.height >= newScreenSize.height)
		{
			scale = entry.scale;
			if (prevScale != scale && onScaleChanged) onScaleChanged(scale);
			return;
		}
	}

	if (scaleList.size() > 0)
	{
		// accept any
		scale = scaleList.begin()->scale;
		if (prevScale != scale && onScaleChanged) onScaleChanged(scale);
	}
}
