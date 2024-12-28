#include "Application/InputManager.h"
#include "Utilities/Logger.h"

void InputManager::SetKey(int key, bool isPressed)
{
    m_KeysCurrent[key] = isPressed;
    Logger::GetLogger()->debug("Key {} is now {}.", key, isPressed ? "Pressed" : "Released");
}

bool InputManager::IsKeyPressed(int key) const
{
    auto it = m_KeysCurrent.find(key);
    return (it != m_KeysCurrent.end()) && it->second;
}

bool InputManager::WasKeyJustPressed(int key) const
{
    bool current = IsKeyPressed(key);
    bool previous = false;
    if (auto it = m_KeysPrevious.find(key); it != m_KeysPrevious.end()) {
        previous = it->second;
    }
    return current && !previous;
}

void InputManager::Update()
{
    m_KeysPrevious = m_KeysCurrent;
}