#pragma once

#include <memory>
#include <string>
#include "Application/InputManager.h"
#include "Camera.h"

namespace Scene {

    /**
     * @class CameraController
     * @brief Handles camera updates based on keyboard and mouse input.
     *
     * This class translates user input from the InputManager into camera movements and rotations.
     */
    class CameraController {
    public:
        /**
         * @brief Constructs a CameraController with a reference to the InputManager.
         * @param inputManager The application-wide InputManager.
         */
        explicit CameraController(InputManager& inputManager);

        /**
         * @brief Updates the camera based on key presses.
         * @param deltaTime Time elapsed (in seconds) since the last frame.
         */
        void Update(float deltaTime);

        /**
         * @brief Processes mouse movement to rotate the camera.
         * @param xpos Current cursor X position.
         * @param ypos Current cursor Y position.
         */
        void ProcessMouseMovement(float xpos, float ypos);

        /**
         * @brief Processes mouse scroll to adjust the camera's FOV.
         * @param yOffset Vertical scroll offset.
         */
        void ProcessMouseScroll(float yOffset);

        /**
         * @brief Sets the camera's movement speed.
         * @param speed New speed in units per second.
         */
        void SetSpeed(float speed);

        /**
         * @brief Resets the mouse state (used when capturing mouse input restarts).
         */
        void Reset();

        /**
         * @brief Requests the camera to update its FOV based on real-world parameters.
         */
        void UpdateFOV();

        /**
         * @brief Associates this controller with a specific Camera.
         * @param camera Shared pointer to a Camera instance.
         */
        void SetCamera(const std::shared_ptr<Camera>& camera);

        /**
         * @brief Checks if a camera is currently assigned.
         * @return True if a camera is assigned, false otherwise.
         */
        bool HasCamera() const;

    private:
        std::shared_ptr<Camera> camera_;
        InputManager& inputManager_;
        float sensitivity_;
        float speed_;
        float lastX_;
        float lastY_;
        bool firstMouse_;
    };

} // namespace Scene