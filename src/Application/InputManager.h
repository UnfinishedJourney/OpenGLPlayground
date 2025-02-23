#pragma once

#include <unordered_map>

/**
 * Manages current and previous key states for keyboard input events.
 */
class InputManager {
public:
    void SetKey(int key, bool isPressed);
    bool IsKeyPressed(int key) const;
    bool WasKeyJustPressed(int key) const;
    void Update();

private:
    std::unordered_map<int, bool> keysCurrent_;
    std::unordered_map<int, bool> keysPrevious_;
};