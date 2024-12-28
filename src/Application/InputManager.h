#pragma once

#include <unordered_map>

/**
 * @brief Manages the current and previous state of keys to detect presses, releases, etc.
 */
class InputManager
{
public:
    void SetKey(int key, bool isPressed);
    bool IsKeyPressed(int key) const;
    bool WasKeyJustPressed(int key) const;
    void Update();

private:
    std::unordered_map<int, bool> m_KeysCurrent;
    std::unordered_map<int, bool> m_KeysPrevious;
};