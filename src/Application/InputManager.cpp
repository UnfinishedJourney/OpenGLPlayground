#include "Application/InputManager.h"
#include "Utilities/Logger.h" 

void InputManager::SetKey(int key, bool isPressed)
{
    m_Keys[key] = isPressed;
    Logger::GetLogger()->info("SetKey called: Key = {}, State = {}.", key, isPressed ? "Pressed" : "Released");
}

bool InputManager::IsKeyPressed(int key) const
{
    auto it = m_Keys.find(key);
    bool isPressed = (it != m_Keys.end()) && it->second;
    Logger::GetLogger()->debug("IsKeyPressed called: Key = {}, IsPressed = {}.", key, isPressed);
    return isPressed;
}