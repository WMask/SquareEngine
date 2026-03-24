/***************************************************************************
* SWorld.h
*/

#pragma once

#include "World/WorldModule.h"
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
	virtual const SWorldScale& GetWorldScale() const { return worldScale; }
	//
	virtual SWorldScale& GetWorldScale() { return worldScale; }
	//
	virtual const class SCamera& GetCamera() const { return camera; }
	//
	virtual class SCamera& GetCamera() override { return camera; }
	//
	virtual void SetGlobalTint(const std::optional<SColor3>& tint) override;
	//
	virtual std::optional<SColor3> GetGlobalTint() const { return globalTint; }


protected:
	//
	SAppContext context;
	//
	entt::registry entities;
	//
	SWorldScale worldScale{};
	//
	SCamera camera{};
	//
	std::optional<SColor3> globalTint;

};
