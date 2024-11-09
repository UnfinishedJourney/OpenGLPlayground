#include "Application/InputManager.h"
#include "Utilities/Logger.h"

void InputManager::SetKey(int key, bool isPressed)
{
    m_KeysCurrent[key] = isPressed;
    Logger::GetLogger()->debug("SetKey called: Key = {}, State = {}.", key, isPressed ? "Pressed" : "Released");
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
    auto it = m_KeysPrevious.find(key);
    if (it != m_KeysPrevious.end()) {
        previous = it->second;
    }
    return current && !previous;
}

void InputManager::Update()
{
    m_KeysPrevious = m_KeysCurrent;
}