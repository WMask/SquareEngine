/***************************************************************************
* SGuiInterface.h
*/

#pragma once

#include "Core/STypes.h"
#include "Application/SApplicationTypes.h"
#include "Application/SLocalizationInterface.h"
#include "RenderSystem/SRenderSystemTypes.h"
#include "RenderSystem/SECSComponents.h"

#include <entt/entt.hpp>


struct SMouseEnterEvent
{
	entt::entity entity;

	SMouseEnterEvent(entt::entity inEntity) : entity(inEntity) {}
};

struct SMouseLeaveEvent
{
	entt::entity entity;

	SMouseLeaveEvent(entt::entity inEntity) : entity(inEntity) {}
};

struct SMouseMoveEvent
{
	entt::entity entity;
	//
	SPoint2 pos;

	SMouseMoveEvent(entt::entity inEntity, SPoint2 inPos) : entity(inEntity), pos(inPos) {}
};

struct SMouseButtonEvent
{
	entt::entity entity;
	//
	SPoint2 pos;
	//
	SMouseBtn btn;
	//
	SKeyState btnState;

	SMouseButtonEvent(entt::entity inEntity, SPoint2 inPos, SMouseBtn inBtn, SKeyState inBtnState)
		: entity(inEntity), pos(inPos), btn(inBtn), btnState(inBtnState)
	{
	}
};

/***************************************************************************
* GUI system interface
*/
class IGuiSystem : public SUncopyable
{
public:
	//
	entt::dispatcher onMouseEvent;

public:
	//
	virtual ~IGuiSystem() {}
	//
	virtual void OnKeys(std::int32_t btn, SKeyState state, const SAppContext& context) = 0;
	//
	virtual void OnMouseButton(SMouseBtn btn, SKeyState state,
		std::int32_t x, std::int32_t y, const SAppContext& context) = 0;
	//
	virtual void OnMouseMove(std::int32_t x, std::int32_t y, const SAppContext& context) = 0;


public:
	//
	virtual entt::entity MakeColoredSprite(entt::registry& registry,
		const SVector3& pos, const SSize2F& size, SColor4F color) = 0;
	//
	virtual entt::entity MakeColoredSprite(
		entt::registry& registry, const SVector3& pos, const SSize2F& size,
		const SColor3& lt, const SColor3& rt, const SColor3& rb, const SColor3& lb) = 0;
	//
	virtual entt::entity MakeTexturedSprite(entt::registry& registry,
		STexID texture, const SVector3& pos, const SSize2F& size, SColor4F color) = 0;
	//
	virtual entt::entity MakeAnimatedSprite(entt::registry& registry,
		STexID texture, const SVector3& pos, const SSize2F& size, SColor4F color,
		std::int32_t frameOffset, std::int32_t framesCount,
		std::int32_t framesPerSecond, SSize2 frameSize, float startTime) = 0;
	//
	virtual entt::entity MakeText(entt::registry& registry, SWidgetID widgetId, STextID text, SFontID font,
		const SVector3& pos, const SSize2F& size, SColor4F color, STextAlign align = STextAlign::Middle) = 0;
	// returns button entity and text entity
	virtual std::pair<entt::entity, entt::entity> MakeButtonWithText(entt::registry& registry,
		STexID texture, STextID text, SFontID font, SWidgetID btnWidget,
		const SVector3& pos, const SSize2F& size, SColor4F color = SConst::White4F, SColor4F textColor = SConst::White4F) = 0;
	//
	virtual entt::entity MakeCheckbox(entt::registry& registry, STexID texture, SWidgetID widgetId,
		bool bChecked, const SVector3& pos, const SSize2F& size, SColor4F color = SConst::White4F) = 0;
	// returns button entity and slider background entity
	virtual std::pair<entt::entity, entt::entity> MakeSlider(entt::registry& registry, STexID texture, SWidgetID widgetId,
		float value, float minValue, float maxValue, const SVector3& pos, const SSize2F& size, const SSize2F& texSize,
		SColor4F color = SConst::White4F) = 0;

};

using TGuiSystemPtr = std::unique_ptr<IGuiSystem>;
