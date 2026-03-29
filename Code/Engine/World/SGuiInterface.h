/***************************************************************************
* SGuiInterface.h
*/

#pragma once

#include "Core/STypes.h"
#include "Application/SApplicationTypes.h"

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
	IGuiSystem() {}
	//
	virtual ~IGuiSystem() {}

};
