/***************************************************************************
* SWorld.h
*/

#pragma once

#include "World/WorldModule.h"
#include "World/SFontSystem.h"
#include "World/SWorldInterface.h"
#include "Application/SApplicationTypes.h"


/***************************************************************************
* Game world class
*/
class SWorld : public IWorld
{
public:
	//
	SWorld(const SAppContext& inContext);


public:// IWorld interface implementation
	//
	virtual ~SWorld() override;
	//
	virtual entt::registry& GetEntities() override { return entities; }
	//
	virtual const entt::registry& GetEntities() const override { return entities; }
	//
	virtual void Clear(bool removeRooted) override;
	//
	virtual std::pair<SLightID, bool> AddDirectionalLight(const std::string_view& name, const SVector3& direction, const SColor3& color) override;
	//
	virtual std::pair<SLightID, bool> AddPointLight(const std::string_view& name, const SVector3& pos, float distance, const SColor3& color) override;
	//
	virtual SLightsBuffer GetLightsData() const override;
	//
	virtual bool RemoveLight(SLightID id) override;
	//
	virtual std::uint32_t GetNumLights(SLightID) const noexcept override { return lights.size(); }
	//
	virtual void UpdateWorldScale(SSize2 newScreenSize) override;
	//
	virtual const SWorldScale& GetScale() const { return worldScale; }
	//
	virtual SWorldScale& GetScale() { return worldScale; }
	//
	virtual const SCamera& GetCamera() const { return camera; }
	//
	virtual SCamera& GetCamera() override { return camera; }
	//
	virtual const IFontSystem& GetFonts() const override { return fonts; }
	//
	virtual IFontSystem& GetFonts() override { return fonts; }
	//
	virtual void SetGlobalTint(const std::optional<SColor3>& tint) override;
	//
	virtual std::optional<SColor3> GetGlobalTint() const { return globalTint; }


protected:
	//
	const SAppContext* context;
	//
	entt::registry entities;
	//
	struct SLight
	{
		// w - light type: <0.5 - directional, >0.5 - point
		SVector4 lightVec;
		// w - distance if point light type
		SVector4 lightColor;
	};
	//
	std::unordered_map<SLightID, SLight> lights;
	//
	SWorldScale worldScale{};
	//
	SCamera camera{};
	//
	SFontSystem fonts;
	//
	std::optional<SColor3> globalTint;

};
