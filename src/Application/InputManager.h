#pragma once

#include <unordered_map>

/**
 * @brief Manages current and previous key states for input detection.
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
