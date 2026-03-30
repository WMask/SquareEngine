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
	virtual void OnMouseButton(SMouseBtn btn, SKeyState state, std::int32_t x, std::int32_t y, const SAppContext& context) override;
	//
	virtual void OnMouseMove(std::int32_t x, std::int32_t y, const SAppContext& context) override;


protected:
	//
	class IWorld* world{};
	//
	class IInputSystem* input{};

};
