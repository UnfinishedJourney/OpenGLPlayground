#pragma once

#include <unordered_map>

class InputManager {
public:
    void SetKey(int key, bool isPressed);
    bool IsKeyPressed(int key) const;

private:
    std::unordered_map<int, bool> m_Keys;
};