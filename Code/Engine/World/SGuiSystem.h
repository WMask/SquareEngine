/***************************************************************************
* SGuiSystem.h
*/

#pragma once

#include "World/WorldModule.h"
#include "World/SGuiInterface.h"


/***************************************************************************
* GUI system class
*/
class S_WORLD_API SGuiSystem : public IGuiSystem
{
public:
	//
	SGuiSystem() {}
	//
	~SGuiSystem() {}
	//
	void Init(class IWorld* world, class IInputSystem* input);
	//
	void OnKeys(std::int32_t btn, SKeyState state, const SAppContext& context);
	//
	void OnMouseButton(SMouseBtn btn, SKeyState state, std::int32_t x, std::int32_t y, const SAppContext& context);
	//
	void OnMouseMove(std::int32_t x, std::int32_t y, const SAppContext& context);


protected:
	//
	class IWorld* world{};
	//
	class IInputSystem* input{};

};
