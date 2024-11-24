#pragma once

#include <unordered_map>

class InputManager {
public:
    void SetKey(int key, bool isPressed);
    bool IsKeyPressed(int key) const;
    bool WasKeyJustPressed(int key) const;
    void Update(); 

private:
    std::unordered_map<int, bool> keysCurrent;
    std::unordered_map<int, bool> keysPrevious;
};