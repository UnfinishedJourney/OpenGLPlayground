#include "Application/InputManager.h"
#include "Utilities/Logger.h"

void InputManager::SetKey(int key, bool isPressed) {
    keysCurrent_[key] = isPressed;
    Logger::GetLogger()->debug("Key {} is {}", key, (isPressed ? "Pressed" : "Released"));
}

bool InputManager::IsKeyPressed(int key) const {
    auto it = keysCurrent_.find(key);
    return (it != keysCurrent_.end()) && it->second;
}

bool InputManager::WasKeyJustPressed(int key) const {
    bool current = IsKeyPressed(key);
    bool previous = false;
    if (auto it = keysPrevious_.find(key); it != keysPrevious_.end()) {
        previous = it->second;
    }
    return (current && !previous);
}

void InputManager::Update() {
    keysPrevious_ = keysCurrent_;
}