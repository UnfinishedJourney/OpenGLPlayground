#pragma once

#include <memory>
#include "Application/InputManager.h"
#include "Scene/Camera.h"

/**
 * @class CameraController
 * @brief Handles camera updates based on keyboard and mouse input.
 *
 * This class mediates between InputManager and the Camera, translating
 * user input into camera movements and rotations.
 */
class CameraController
{
public:
    /**
     * @brief Constructs a CameraController that references a given InputManager.
     *
     * @param inputManager A reference to the global or application-wide InputManager.
     */
    explicit CameraController(InputManager& inputManager);

    /**
     * @brief Updates the camera position based on key presses (WASD, arrow keys).
     *
     * @param deltaTime Time elapsed (in seconds) since the last frame.
     */
    void Update(float deltaTime);

    /**
     * @brief Processes mouse movement (xpos/ypos) to rotate the camera.
     *
     * @param xpos  Current cursor X-position.
     * @param ypos  Current cursor Y-position.
     */
    void ProcessMouseMovement(float xpos, float ypos);

    /**
     * @brief Processes mouse wheel/scroll input to zoom (adjust FOV).
     *
     * @param yOffset The scroll offset in the vertical direction.
     */
    void ProcessMouseScroll(float yOffset);

    /**
     * @brief Sets the movement speed that is applied to the camera.
     *
     * @param speed New movement speed (units per second).
     */
    void SetSpeed(float speed);

    /**
     * @brief Resets mouse state, typically called when you stop capturing mouse movement.
     */
    void Reset();

    /**
     * @brief Requests the camera to update its FOV based on real-world data.
     */
    void UpdateFOV();

    /**
     * @brief Associates the controller with a specific Camera.
     *
     * @param camera The shared pointer to a Camera instance.
     */
    void SetCamera(const std::shared_ptr<Camera>& camera) { m_Camera = camera; }

    /**
     * @brief Checks if we have a valid camera reference.
     *
     * @return True if a camera is currently assigned, false otherwise.
     */
    bool HasCamera() const { return (m_Camera != nullptr); }

private:
    /**
     * @brief The camera instance this controller manipulates.
     */
    std::shared_ptr<Camera> m_Camera;

    /**
     * @brief Reference to the global or application-wide InputManager.
     */
    InputManager& m_InputManager;

    /**
     * @brief Mouse rotation sensitivity factor.
     */
    float m_Sensitivity;

    /**
     * @brief Movement speed (units per second).
     */
    float m_Speed;

    /**
     * @brief Tracks the mouse's last X position for calculating xOffset.
     */
    float m_LastX;

    /**
     * @brief Tracks the mouse's last Y position for calculating yOffset.
     */
    float m_LastY;

    /**
     * @brief True if the mouse movement logic should initialize (first mouse event).
     */
    bool m_FirstMouse;
};