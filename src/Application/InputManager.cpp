#include "Application/InputManager.h"

void InputManager::SetKey(int key, bool isPressed) {
    m_Keys[key] = isPressed;
}

bool InputManager::IsKeyPressed(int key) const {
    auto it = m_Keys.find(key);
    return it != m_Keys.end() && it->second;
}