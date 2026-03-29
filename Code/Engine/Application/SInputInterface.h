/***************************************************************************
* SInputInterface.h
*/

#pragma once

#include "Application/SApplicationTypes.h"
#include "Application/SAppConfig.h"

#include <functional>
#include <memory>
#include <deque>

#ifdef _WINDOWS
# include <windows.h>
#endif

#if defined(_MSC_VER)
# pragma warning(disable : 4275)
# pragma warning(disable : 4251)
#endif


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

namespace SConst
{
	static const std::uint32_t KeysCount = 150;
	static const std::uint32_t JoysticksMaxCount = 4;
	static const std::uint32_t JoystickKeysOffset = 130;
	static const std::uint32_t MouseKeysCount = 3;
	static const std::uint32_t JoystickKeysCount = (KeysCount - JoystickKeysOffset);
	static const std::uint8_t FirstJoystickId = static_cast<uint8_t>(SInputDeviceType::Joystick1);
}

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
	uint8_t keys[SConst::KeysCount];
};

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
	static const int X				= SConst::JoystickKeysOffset + 0;
	static const int A				= SConst::JoystickKeysOffset + 1;
	static const int B				= SConst::JoystickKeysOffset + 2;
	static const int Y				= SConst::JoystickKeysOffset + 3;
	static const int L1				= SConst::JoystickKeysOffset + 4;
	static const int R1				= SConst::JoystickKeysOffset + 5;
	static const int L2				= SConst::JoystickKeysOffset + 6;
	static const int R2				= SConst::JoystickKeysOffset + 7;
	static const int Back			= SConst::JoystickKeysOffset + 8;
	static const int Menu			= SConst::JoystickKeysOffset + 9;
	static const int Start			= SConst::JoystickKeysOffset + 9;
	static const int Up				= SConst::JoystickKeysOffset + 10;
	static const int Right			= SConst::JoystickKeysOffset + 11;
	static const int Down			= SConst::JoystickKeysOffset + 12;
	static const int Left			= SConst::JoystickKeysOffset + 13;
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
