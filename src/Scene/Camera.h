#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

namespace Scene {

    /**
     * @enum CameraMovement
     * @brief Enumerates possible movement directions for the camera.
     */
    enum class CameraMovement {
        Forward,
        Backward,
        Left,
        Right,
        Up,
        Down
    };

    /**
     * @class Camera
     * @brief Encapsulates view/projection matrices and controls for a 3D camera.
     *
     * This camera uses a position, yaw, and pitch to define its orientation,
     * and calculates projection matrices based on the current screen size.
     */
    class Camera {
    public:
        /**
         * @brief Constructs a camera with given position, up vector, yaw, and pitch.
         * @param position Initial position in world space (default: (0, 0, 8)).
         * @param up World-space up vector (default: (0, 1, 0)).
         * @param yaw Initial yaw angle in degrees (default: -90).
         * @param pitch Initial pitch angle in degrees (default: 0).
         */
        Camera(const glm::vec3& position = glm::vec3(0.0f, 0.0f, 8.0f),
            const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f),
            float yaw = -90.0f,
            float pitch = 0.0f);

        /**
         * @brief Returns the view matrix computed from the camera's position and orientation.
         */
        glm::mat4 GetViewMatrix() const;

        /**
         * @brief Returns the current projection matrix.
         */
        glm::mat4 GetProjectionMatrix() const;

        /**
         * @brief Gets the current vertical field of view (in degrees).
         */
        float GetFOV() const;

        /**
         * @brief Sets the field-of-view and updates the projection matrix.
         * @param fov New FOV in degrees.
         */
        void SetFOV(float fov);

        /**
         * @brief Moves the camera in the specified direction based on its speed and deltaTime.
         * @param direction The desired CameraMovement direction.
         * @param deltaTime Time elapsed (in seconds) since the last frame.
         */
        void Move(CameraMovement direction, float deltaTime);

        /**
         * @brief Rotates the camera by the given yaw and pitch offsets.
         * @param xOffset Change in yaw (degrees).
         * @param yOffset Change in pitch (degrees).
         */
        void Rotate(float xOffset, float yOffset);

        /**
         * @brief Sets the movement speed (units per second).
         * @param speed New movement speed.
         */
        void SetSpeed(float speed);

        /**
         * @brief Dynamically updates the FOV based on real-world display parameters.
         */
        void UpdateFOV();

        /**
         * @brief Recomputes the projection matrix using a given aspect ratio.
         * @param aspectRatio Width/height ratio.
         */
        void UpdateProjectionMatrix(float aspectRatio);

        /// @brief Returns the camera's position.
        glm::vec3 GetPosition() const { return position_; }

        /// @brief Returns a modifiable reference to the camera's position.
        glm::vec3& GetPositionRef() { return position_; }

        /// @brief Returns the camera's forward (front) vector.
        glm::vec3 GetFront() const { return front_; }

        /// @brief Returns the camera's up vector.
        glm::vec3 GetUp() const { return up_; }

        /// @brief Gets the near clipping plane.
        float GetNearPlane() const;

        /// @brief Sets the near clipping plane and updates the projection matrix.
        void SetNearPlane(float nearPlane);

        /// @brief Gets the far clipping plane.
        float GetFarPlane() const;

        /// @brief Sets the far clipping plane and updates the projection matrix.
        void SetFarPlane(float farPlane);

        /// @brief Sets the camera's position.
        void SetPosition(const glm::vec3& pos) { position_ = pos; }

    private:
        /// @brief Recalculates the front, right, and up vectors from yaw and pitch.
        void UpdateCameraVectors();

    private:
        // Camera parameters
        glm::vec3 position_;     ///< Current position in world space.
        glm::vec3 front_;        ///< Forward direction vector (normalized).
        glm::vec3 up_;           ///< Up direction vector.
        glm::vec3 right_;        ///< Right direction vector.
        glm::vec3 worldUp_;      ///< World up vector (remains constant).

        float yaw_;              ///< Yaw angle (in degrees).
        float pitch_;            ///< Pitch angle (in degrees).
        float fov_;              ///< Field of view (in degrees).
        float speed_;            ///< Movement speed (units per second).
        float mouseSensitivity_; ///< Mouse sensitivity for rotation.
        float nearPlane_;        ///< Near clipping plane.
        float farPlane_;         ///< Far clipping plane.

        // Matrices
        glm::mat4 projectionMatrix_; ///< Current perspective projection matrix.
    };

} // namespace Scene