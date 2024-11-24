#include "Application/InputManager.h"
#include "Utilities/Logger.h"

void InputManager::SetKey(int key, bool isPressed)
{
    keysCurrent[key] = isPressed;
    Logger::GetLogger()->debug("Key {} is now {}.", key, isPressed ? "Pressed" : "Released");
}

bool InputManager::IsKeyPressed(int key) const
{
    auto it = keysCurrent.find(key);
    return (it != keysCurrent.end()) && it->second;
}

bool InputManager::WasKeyJustPressed(int key) const
{
    bool current = IsKeyPressed(key);
    bool previous = keysPrevious.count(key) ? keysPrevious.at(key) : false;
    return current && !previous;
}

void InputManager::Update()
{
    keysPrevious = keysCurrent;
}