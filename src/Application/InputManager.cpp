#include "Application/InputManager.h"
#include "Utilities/Logger.h"

void InputManager::SetKey(int key, bool isPressed)
{
    m_Keys[key] = isPressed;
    Logger::GetLogger()->debug("SetKey called: Key = {}, State = {}.", key, isPressed ? "Pressed" : "Released");
}

bool InputManager::IsKeyPressed(int key) const
{
    auto it = m_Keys.find(key);
    return (it != m_Keys.end()) && it->second;
}