#pragma once

#include <unordered_map>

/**
 * @class InputManager
 * @brief Manages current and previous key states for detecting keyboard input events.
 *
 * This class provides methods to update key press states, query if a key is currently
 * pressed, and detect a "just pressed" event (i.e., pressed this frame but not last frame).
 */
class InputManager
{
public:
    /**
     * @brief Sets the current press state of a specific key.
     *
     * @param key       The integer key code (e.g., GLFW_KEY_*).
     * @param isPressed True if the key is currently pressed, false if released.
     */
    void SetKey(int key, bool isPressed);

    /**
     * @brief Checks if a key is currently pressed.
     *
     * @param key The integer key code to query.
     * @return True if the key is pressed, false otherwise.
     */
    bool IsKeyPressed(int key) const;

    /**
     * @brief Checks if a key was pressed this frame, but not in the previous frame.
     *
     * @param key The integer key code to query.
     * @return True if the key transitioned from "not pressed" to "pressed" this frame.
     */
    bool WasKeyJustPressed(int key) const;

    /**
     * @brief Updates the previous frame's key states to match the current frame's states.
     *
     * This should be called once per frame (after handling input events)
     * to allow detection of key transition events.
     */
    void Update();

private:
    /**
     * @brief Maps key codes to their current pressed state.
     */
    std::unordered_map<int, bool> m_KeysCurrent;

    /**
     * @brief Maps key codes to their pressed state from the previous frame.
     */
    std::unordered_map<int, bool> m_KeysPrevious;
};