/***************************************************************************
* SInputSystem.h
*/

#pragma once

#include "Application/ApplicationModule.h"
#include "Application/SApplicationTypes.h"
#include "Application/SAppConfig.h"

#include <functional>
#include <memory>

#ifdef _WINDOWS
# include <windows.h>
#endif

#if defined(_MSC_VER)
# pragma warning(disable : 4275)
# pragma warning(disable : 4251)
#endif


static const int SKeysCount = 150;
static const int SJoysticksMaxCount = 4;
static const int SJoystickKeysOffset = 130;
static const int SMouseKeysCount = 3;
static const int SJoystickKeysCount = (SKeysCount - SJoystickKeysOffset);


/** Input device type */
enum class SInputDeviceType : uint8_t
{
	Keyboard,
	Mouse,
	Joystick1,
	Joystick2,
	Joystick3,
	Joystick4
};

static constexpr uint8_t FirstJoystickId = static_cast<uint8_t>(SInputDeviceType::Joystick1);

/** Keys struct */
struct KEYS
{
	KEYS();
	//
	uint8_t* Get() { return keys; }
	//
	const uint8_t* Get() const { return keys; }
	// throws
	uint8_t& at(int index);
	//
	uint8_t& operator[](int index) noexcept { return keys[index]; }
	//
	uint8_t keys[SKeysCount];
};

/** Mouse buttons */
namespace SMouseBtn
{
	static const int Left   = 0;
	static const int Right  = 1;
	static const int Middle = 2;
}

/** Keyboard keys */
namespace SKeys
{
#ifdef _WINDOWS

	static const int Escape = VK_ESCAPE;
	static const int Space  = VK_SPACE;
	static const int Return = VK_RETURN;
	static const int Enter  = VK_RETURN;
	static const int Up     = VK_UP;
	static const int Right  = VK_RIGHT;
	static const int Down   = VK_DOWN;
	static const int Left   = VK_LEFT;

#endif
}

/** Joystick keys */
namespace SJKeys
{
	static const int X				= SJoystickKeysOffset + 0;
	static const int A				= SJoystickKeysOffset + 1;
	static const int B				= SJoystickKeysOffset + 2;
	static const int Y				= SJoystickKeysOffset + 3;
	static const int L1				= SJoystickKeysOffset + 4;
	static const int R1				= SJoystickKeysOffset + 5;
	static const int L2				= SJoystickKeysOffset + 6;
	static const int R2				= SJoystickKeysOffset + 7;
	static const int Back			= SJoystickKeysOffset + 8;
	static const int Menu			= SJoystickKeysOffset + 9;
	static const int Start			= SJoystickKeysOffset + 9;
	static const int Up				= SJoystickKeysOffset + 10;
	static const int Right			= SJoystickKeysOffset + 11;
	static const int Down			= SJoystickKeysOffset + 12;
	static const int Left			= SJoystickKeysOffset + 13;
	static const int StartArrows	= Up;
	static const int EndArrows		= Left;
}

/** Joystick axis */
namespace SJAxis
{
	static const int LStick = 0;
	static const int RStick = 1;
}

/** Action type */
enum class SActionType
{
	Key,
	Mouse,
	Axis
};

/** Action base */
struct SAction
{
	SAction(const std::string& inName, SActionType inType, SKeyState inState = SKeyState::Down)
		: name(inName)
		, type(inType)
		, state(inState) {}
	//
	inline bool Pressed(const std::string& inName) const { return (name == inName) && (state == SKeyState::Down); }
	//
	inline bool Released(const std::string& inName) const { return (name == inName) && (state == SKeyState::Up); }
	//
	std::string name;
	//
	SActionType type;
	//
	SKeyState state;
};

/** Key action */
struct SKeyAction : public SAction
{
	SKeyAction(const std::string& inName, int inKey, SKeyState inState)
		: SAction(inName, SActionType::Key, inState)
		, key(inKey) {}
	// LcKeys or LcJKeys
	int key;
};

/** Mouse action */
struct SMouseAction : public SAction
{
	SMouseAction(const std::string& inName, int inButton, SKeyState inState, float inX, float inY)
		: SAction(inName, SActionType::Mouse, inState)
		, button(inButton)
		, x(inX), y(inY) {}
	// LcMouseBtn
	int button;
	//
	float x;
	//
	float y;
};

/** Axis action */
struct SAxisAction : public SAction
{
	SAxisAction(const std::string& inName, int inAxis, float inX, float inY)
		: SAction(inName, SActionType::Axis)
		, axis(inAxis)
		, x(inX), y(inY) {}
	// LcJAxis
	int axis;
	//
	float x;
	//
	float y;
};


/**
* Input device interface */
class IInputDevice
{
public:
	static const std::wstring MouseAndKeyboard;


public:
	/**
	* Virtual destructor */
	virtual ~IInputDevice() {}
	/**
	* Set as active */
	virtual void Activate() = 0;
	/**
	* Set as inactive */
	virtual void Deactivate() = 0;
	/**
	* Update device */
	virtual void Update(float deltaSeconds, SAppContext context) = 0;
	/**
	* Get action state */
	virtual bool Pressed(const std::string& actionName) const = 0;
	/**
	* Get active state */
	virtual bool IsActive() const noexcept = 0;
	/**
	* Set input state */
	virtual void SetState(int key, bool bPressed) = 0;
	/**
	* Get input state */
	virtual const KEYS& GetState() const noexcept = 0;
	/**
	* Get previous input state */
	virtual const KEYS& GetPrevState() const noexcept = 0;
	/**
	* Get joystick name */
	virtual std::wstring GetName() const = 0;
	/**
	* Get device type */
	virtual SInputDeviceType GetType() const noexcept = 0;
	/**
	* Get pointer position */
	virtual SVector2 GetPointerPos() const noexcept { return SVector2{}; }
	/**
	* Get previous pointer position */
	virtual SVector2 GetPrevPointerPos() const noexcept { return SVector2{}; }
	/**
	* Set pointer position */
	virtual void SetPointerPos(SVector2 pos) noexcept {}

};


/** Input devices list */
using TInputDevicesList = std::deque<std::unique_ptr<IInputDevice>>;

/** Keyboard events handler */
using SKeysHandler = std::function<void(std::int32_t, SKeyState, SAppContext)>;

/** Mouse button handler */
using SMouseButtonHandler = std::function<void(std::int32_t, SKeyState, float, float, SAppContext)>;

/** Mouse move handler */
using SMouseMoveHandler = std::function<void(float, float, SAppContext)>;

/** Action handler */
using SActionHandler = std::function<void(const SAction& action, SAppContext)>;

/** Gamepad axis events handler */
using SAxisHandler = std::function<void(std::int32_t, float, float, SAppContext)>;


/**
* Input system */
class IInputSystem
{
public:
	/**
	* Virtual destructor */
	virtual ~IInputSystem() {}
	/**
	* Initialize input system */
	virtual void Init(const SAppContext& context) = 0;
	/**
	* Shutdown input system */
	virtual void Shutdown() = 0;
	/**
	* Update input system */
	virtual void Update(float deltaSeconds, SAppContext context) = 0;
	/**
	* Set active device */
	virtual void SetActiveDevice(const IInputDevice* device) = 0;
	/**
	* Set active device by name part. Like "MyGamepad" for "Wireless MyGamepad Controller" */
	virtual void SetActiveDevice(const std::wstring& deviceNamePart) = 0;
	/**
	* Set keyboard handler */
	virtual void SetKeysHandler(SKeysHandler handler) noexcept = 0;
	/**
	* Set action handler */
	virtual void SetActionHandler(SActionHandler handler) noexcept = 0;
	/**
	* Set mouse move handler */
	virtual void SetMouseMoveHandler(SMouseMoveHandler handler) noexcept = 0;
	/**
	* Set mouse button handler */
	virtual void SetMouseButtonHandler(SMouseButtonHandler handler) noexcept = 0;
	/**
	* Set gamepad axis handler */
	virtual void SetAxisHandler(SAxisHandler handler) noexcept = 0;
	/**
	* Get keyboard handler */
	virtual SKeysHandler& GetKeysHandler() noexcept = 0;
	/**
	* Get action handler */
	virtual SActionHandler& GetActionHandler() noexcept = 0;
	/**
	* Get mouse move handler */
	virtual SMouseMoveHandler& GetMouseMoveHandler() noexcept = 0;
	/**
	* Get mouse button handler */
	virtual SMouseButtonHandler& GetMouseButtonHandler() noexcept = 0;
	/**
	* Get gamepad axis handler */
	virtual SAxisHandler& GetAxisHandler() noexcept = 0;
	/**
	* Get input devices list */
	virtual const TInputDevicesList& GetInputDevicesList() const noexcept = 0;
	/**
	* Get input devices list */
	virtual TInputDevicesList& GetInputDevicesList() noexcept = 0;
	/**
	* Get active input device */
	virtual const IInputDevice* GetActiveInputDevice() const noexcept = 0;
	/**
	* Get active input device */
	virtual IInputDevice* GetActiveInputDevice() noexcept = 0;

};

using TInputSystemPtr = std::unique_ptr<IInputSystem>;


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
