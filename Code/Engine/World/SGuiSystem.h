/***************************************************************************
* SGuiSystem.h
*/

#pragma once

#include "World/SGuiInterface.h"


/***************************************************************************
* GUI system class
*/
class SGuiSystem : public IGuiSystem
{
public:
	//
	SGuiSystem() {}
	//
	void Init(class IWorld* world, class IInputSystem* input);


public: // IGuiSystem interface implementation
	//
	virtual ~SGuiSystem() {}
	//
	virtual void OnKeys(std::int32_t btn, SKeyState state, const SAppContext& context) override;
	//
	virtual void OnMouseButton(SMouseBtn btn, SKeyState state,
		std::int32_t x, std::int32_t y, const SAppContext& context) override;
	//
	virtual void OnMouseMove(std::int32_t x, std::int32_t y, const SAppContext& context) override;
	//
	virtual entt::entity MakeColoredSprite(entt::registry& registry,
		const SVector3& pos, const SSize2F& size, SColor4F color) override;
	//
	virtual entt::entity MakeColoredSprite(
		entt::registry& registry, const SVector3& pos, const SSize2F& size,
		const SColor3& lt, const SColor3& rt, const SColor3& rb, const SColor3& lb) override;
	//
	virtual entt::entity MakeTexturedSprite(entt::registry& registry,
		STexID texture, const SVector3& pos, const SSize2F& size, SColor4F color) override;
	//
	virtual entt::entity MakeAnimatedSprite(entt::registry& registry,
		STexID texture, const SVector3& pos, const SSize2F& size, SColor4F color,
		std::int32_t frameOffset, std::int32_t framesCount,
		std::int32_t framesPerSecond, SSize2 frameSize, float startTime) override;
	//
	virtual entt::entity MakeText(entt::registry& registry, SWidgetID widgetId, STextID text, SFontID font,
		const SVector3& pos, const SSize2F& size, SColor4F color, STextAlign align) override;
	//
	virtual std::pair<entt::entity, entt::entity> MakeButtonWithText(entt::registry& registry,
		STexID texture, STextID text, SFontID font, SWidgetID btnWidget, SWidgetID textWidget,
		const SVector3& pos, const SSize2F& size, SColor4F color) override;


protected:
	//
	class IWorld* world{};
	//
	class IInputSystem* input{};

};
