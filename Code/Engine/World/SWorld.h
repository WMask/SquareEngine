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
	SWorldScale worldScale{};
	//
	SCamera camera{};
	//
	SFontSystem fonts;
	//
	std::optional<SColor3> globalTint;

};
