#include "Application/InputManager.h"
#include "Utilities/Logger.h"

void InputManager::SetKey(int key, bool isPressed)
{
    m_KeysCurrent[key] = isPressed;

    // Optional: log at "trace" or "debug" level if needed.
    Logger::GetLogger()->debug("Key {} is {}", key, (isPressed ? "Pressed" : "Released"));
}

bool InputManager::IsKeyPressed(int key) const
{
    // Check if key exists in the current map and return its state
    auto it = m_KeysCurrent.find(key);
    return (it != m_KeysCurrent.end()) && it->second;
}

bool InputManager::WasKeyJustPressed(int key) const
{
    bool current = IsKeyPressed(key);

    // Default to false if not found in previous map
    bool previous = false;
    if (auto it = m_KeysPrevious.find(key); it != m_KeysPrevious.end()) {
        previous = it->second;
    }

    return (current && !previous);
}

void InputManager::Update()
{
    // Copy current state into previous state for the next frame
    m_KeysPrevious = m_KeysCurrent;
}