/***************************************************************************
* SInputSystem.cpp
*/

#include "Application/SInputSystem.h"
#include "Application/SApplicationInterface.h"
#include "Core/SException.h"


const std::wstring IInputDevice::MouseAndKeyboard = L"[MK]";
const std::wstring SKeyboardInputDevice::Name = L"[MK] Keyboard";
const std::wstring SMouseInputDevice::Name = L"[MK] Mouse";

SDefaultInputSystem::SDefaultInputSystem() : activeDevice(nullptr), cfg(nullptr)
{
}

void SDefaultInputSystem::Init(const SAppContext& context)
{
    cfg = context.app ? &context.app->GetConfig() : nullptr;

#ifdef _WINDOWS

    devices.push_back(std::make_unique<SKeyboardInputDevice>(cfg));
    devices.push_back(std::make_unique<SMouseInputDevice>(cfg));
    activeDevice = devices[0].get();

#endif
}

std::deque<SActionBinding> GetActions(SActionType type, int id, const SAppConfig& cfg)
{
    std::deque<SActionBinding> actions;

    for (auto& action : cfg.Actions)
    {
        switch (type)
        {
        case SActionType::Key:
            if (id == action.Key) actions.push_back(action);
            break;
        case SActionType::Mouse:
            if (id == action.MouseBtn) actions.push_back(action);
            break;
        case SActionType::Axis:
            break;
        }
    }

    return actions;
}

void SDefaultInputSystem::Update(float deltaSeconds, SAppContext context)
{
    for (auto& device : devices)
    {
        uint8_t deviceId = static_cast<uint8_t>(device->GetType());
        if (!device->IsActive() || deviceId >= SConst::FirstJoystickId) continue;

        if (device->GetType() == SInputDeviceType::Keyboard)
        {
            for (int key = 0; key < SConst::JoystickKeysOffset; key++)
            {
                bool bIsPressed = device->GetState().keys[key] && !device->GetPrevState().keys[key];
                bool bIsReleased = !device->GetState().keys[key] && device->GetPrevState().keys[key];

                if (bIsPressed || bIsReleased)
                {
                    SKeyState keyState = bIsPressed ? SKeyState::Down : SKeyState::Up;

                    if (keysHandler) keysHandler(key, keyState, context);
                    if (actionHandler && cfg)
                    {
                        auto actions = GetActions(SActionType::Key, key, *cfg);
                        for (auto& action : actions)
                        {
                            actionHandler(SKeyAction(action.Name, key, keyState), context);
                        }
                    }
                }
            }
        }
        else if (device->GetType() == SInputDeviceType::Mouse)
        {
            SVector2 mousePos = device->GetPointerPos();
            SPoint2 mousePosInt {
                static_cast<int>(std::round(mousePos.x) + 0.1f),
                static_cast<int>(std::round(mousePos.y) + 0.1f)
            };

            for (int button = 0; button < SConst::MouseKeysCount; button++)
            {
                bool bIsPressed = device->GetState().keys[button] && !device->GetPrevState().keys[button];
                bool bIsReleased = !device->GetState().keys[button] && device->GetPrevState().keys[button];

                if (bIsPressed || bIsReleased)
                {
                    SKeyState buttonState = bIsPressed ? SKeyState::Down : SKeyState::Up;

                    if (mouseButtonHandler) mouseButtonHandler(button, buttonState, mousePos.x, mousePos.y, context);
                    if (actionHandler && cfg)
                    {
                        auto actions = GetActions(SActionType::Mouse, button, *cfg);
                        for (auto& action : actions)
                        {
                            actionHandler(SMouseAction(action.Name, button, buttonState, mousePos.x, mousePos.y), context);
                        }
                    }
                }
            }

            if (device->GetPointerPos() != device->GetPrevPointerPos())
            {
                if (mouseMoveHandler) mouseMoveHandler(mousePos.x, mousePos.y, context);
            }
        }

        device->Update(deltaSeconds, context);
    }
}

void SDefaultInputSystem::SetActiveDevice(const std::wstring& deviceNamePart)
{
    for (auto& device : devices)
    {
        auto deviceName = device->GetName();
        if (deviceName.find(deviceNamePart) != deviceName.npos)
        {
            activeDevice = device.get();
            activeDevice->Activate();
        }
        else
        {
            device->Deactivate();
        }
    }
}

void SDefaultInputSystem::SetActiveDevice(const IInputDevice* inActiveDevice)
{
    for (auto& device : devices)
    {
        if (device.get() == inActiveDevice)
        {
            activeDevice = device.get();
            activeDevice->Activate();
        }
        else
        {
            device->Deactivate();
        }
    }
}

void SKeyboardInputDevice::Update(float deltaSeconds, SAppContext context)
{
    memcpy(prevKeys.Get(), keys.Get(), sizeof(keys));
}

bool SKeyboardInputDevice::Pressed(const std::string& actionName) const
{
    if (!cfg || !active) return false;

    for (auto& action : cfg->Actions)
    {
        if (action.Name == actionName)
        {
            if (action.Key >= 0 && action.Key < SConst::JoystickKeysOffset && (keys.keys[action.Key] != 0))
            {
                return true;
            }

            if (action.JoyKey >= SConst::JoystickKeysOffset && action.JoyKey < SConst::KeysCount && (keys.keys[action.JoyKey] != 0))
            {
                return true;
            }

            break;
        }
    }

    return false;
}

void SMouseInputDevice::Update(float deltaSeconds, SAppContext context)
{
    memcpy(prevButtons.Get(), buttons.Get(), sizeof(buttons));
    prevMousePos = mousePos;
}

bool SMouseInputDevice::Pressed(const std::string& actionName) const
{
    if (!cfg || !active) return false;

    for (auto& action : cfg->Actions)
    {
        if (action.Name == actionName)
        {
            if (action.MouseBtn >= 0 && action.MouseBtn < SConst::MouseKeysCount && (buttons.keys[action.MouseBtn] != 0))
            {
                return true;
            }

            break;
        }
    }

    return false;
}

KEYS::KEYS()
{
    memset(keys, 0, sizeof(keys));
}

uint8_t& KEYS::at(int index)
{
    if (index < 0 || index >= SConst::KeysCount) throw SException("KEYS::operator[]: Invalid index");

    return keys[index];
}


TInputSystemPtr CreateDefaultInputSystem()
{
    return std::make_unique<SDefaultInputSystem>();
}
