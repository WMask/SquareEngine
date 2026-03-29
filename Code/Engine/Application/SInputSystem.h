/***************************************************************************
* SInputSystem.h
*/

#pragma once

#include "Application/SInputInterface.h"


/**
* Default input device with keys */
class SKeyboardInputDevice : public IInputDevice
{
public:
	static const std::wstring Name;


public:
	SKeyboardInputDevice(const SAppConfig* inCfg) : cfg(inCfg), name(Name), active(true) {}


public:
	//
	virtual ~SKeyboardInputDevice() {}
	//
	virtual void Activate() override { active = true; }
	//
	virtual void Deactivate() override { active = false; }
	//
	virtual void Update(float deltaSeconds, SAppContext context) override;
	//
	virtual bool Pressed(const std::string& actionName) const override;
	//
	virtual bool IsActive() const noexcept override { return active; }
	//
	virtual void SetState(std::int32_t key, bool bPressed) override { keys[key] = bPressed; }
	//
	virtual const KEYS& GetState() const noexcept override { return keys; }
	//
	virtual const KEYS& GetPrevState() const noexcept override { return prevKeys; }
	//
	virtual std::wstring GetName() const override { return name; }
	//
	virtual SInputDeviceType GetType() const noexcept override { return SInputDeviceType::Keyboard; }


protected:
	//
	KEYS keys;
	//
	KEYS prevKeys;
	//
	std::wstring name;
	//
	const SAppConfig* cfg;
	//
	bool active;

};


/**
* Default input device with pointer */
class SMouseInputDevice : public IInputDevice
{
public:
	static const std::wstring Name;


public:
	//
	SMouseInputDevice(const SAppConfig* inCfg) : cfg(inCfg), name(Name), mousePos{}, active(true) {}


public:
	//
	virtual ~SMouseInputDevice() {}
	//
	virtual void Activate() override { active = true; }
	//
	virtual void Deactivate() override { active = false; }
	//
	virtual void Update(float deltaSeconds, SAppContext context) override;
	//
	virtual bool Pressed(const std::string& actionName) const override;
	//
	virtual bool IsActive() const noexcept override { return active; }
	//
	virtual void SetState(std::int32_t key, bool bPressed) override { buttons[key] = bPressed; }
	//
	virtual const KEYS& GetState() const noexcept override { return buttons; }
	//
	virtual const KEYS& GetPrevState() const noexcept override { return prevButtons; }
	//
	virtual std::wstring GetName() const override { return name; }
	//
	virtual SInputDeviceType GetType() const noexcept override { return SInputDeviceType::Mouse; }
	//
	virtual SVector2 GetPointerPos() const noexcept override { return mousePos; }
	//
	virtual SVector2 GetPrevPointerPos() const noexcept override { return prevMousePos; }
	//
	virtual void SetPointerPos(SVector2 pos) noexcept override { mousePos = pos; }


protected:
	//
	KEYS buttons;
	//
	KEYS prevButtons;
	//
	SVector2 mousePos;
	//
	SVector2 prevMousePos{};
	//
	std::wstring name;
	//
	const SAppConfig* cfg;
	//
	bool active;

};


/**
* Default Input system */
class SDefaultInputSystem : public IInputSystem
{
public:
	//
	SDefaultInputSystem();


public: // IInputSystem interface implementation
	//
	virtual ~SDefaultInputSystem() override {}
	//
	virtual void Init(const SAppContext& context) override;
	//
	virtual void Shutdown() override {}
	//
	virtual void Update(float deltaSeconds, SAppContext context) override;
	//
	virtual void SetActiveDevice(const std::wstring& deviceNamePart) override;
	//
	virtual void SetActiveDevice(const IInputDevice* device) override;
	//
	virtual void SetKeysHandler(SKeysHandler handler) noexcept override { keysHandler = handler; }
	//
	virtual void SetActionHandler(SActionHandler handler) noexcept override { actionHandler = handler; }
	//
	virtual void SetMouseMoveHandler(SMouseMoveHandler handler) noexcept override { mouseMoveHandler = handler; }
	//
	virtual void SetMouseButtonHandler(SMouseButtonHandler handler) noexcept override { mouseButtonHandler = handler; }
	//
	virtual void SetAxisHandler(SAxisHandler handler) noexcept override { axisHandler = handler; }
	//
	virtual SKeysHandler& GetKeysHandler() noexcept override { return keysHandler; }
	//
	virtual SActionHandler& GetActionHandler() noexcept override { return actionHandler; }
	//
	virtual SMouseMoveHandler& GetMouseMoveHandler() noexcept override { return mouseMoveHandler; }
	//
	virtual SMouseButtonHandler& GetMouseButtonHandler() noexcept override { return mouseButtonHandler; }
	//
	virtual SAxisHandler& GetAxisHandler() noexcept override { return axisHandler; }
	//
	virtual const TInputDevicesList& GetInputDevicesList() const noexcept override { return devices; }
	//
	virtual TInputDevicesList& GetInputDevicesList() noexcept override { return devices; }
	//
	virtual const IInputDevice* GetActiveInputDevice() const noexcept override { return activeDevice; }
	//
	virtual IInputDevice* GetActiveInputDevice() noexcept override { return activeDevice; }


protected:
	//
	SKeysHandler keysHandler;
	//
	SActionHandler actionHandler;
	//
	SMouseMoveHandler mouseMoveHandler;
	//
	SMouseButtonHandler mouseButtonHandler;
	//
	SAxisHandler axisHandler;
	//
	IInputDevice* activeDevice;
	//
	TInputDevicesList devices;
	//
	const SAppConfig* cfg;

};


/**
* Default input system */
S_APPLICATION_API TInputSystemPtr CreateDefaultInputSystem();
