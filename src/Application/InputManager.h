#pragma once

#include <unordered_map>

class InputManager {
public:
    void SetKey(int key, bool isPressed);
    bool IsKeyPressed(int key) const;
    bool WasKeyJustPressed(int key) const;
    void Update(); // Call this at the end of each frame

private:
    std::unordered_map<int, bool> m_KeysCurrent;
    std::unordered_map<int, bool> m_KeysPrevious;
};