#pragma once

#include <glm/glm.hpp>

/**
 * @enum CameraMovement
 * @brief Enum describing possible movement directions for the camera.
 */
enum class CameraMovement
{
    Forward,
    Backward,
    Left,
    Right,
    Up,
    Down
};

/**
 * @class Camera
 * @brief Encapsulates the view/projection matrices and controls for a 3D camera.
 *
 * This camera uses a position, yaw, and pitch to define its orientation,
 * and calculates projection matrices based on the current screen size.
 */
class Camera
{
public:
    /**
     * @brief Constructs a camera with given position, up vector, yaw, and pitch.
     *
     * @param position Initial position of the camera in world space.
     * @param up       World-space up vector.
     * @param yaw      Initial yaw angle (in degrees).
     * @param pitch    Initial pitch angle (in degrees).
     */
    Camera(const glm::vec3& position = glm::vec3(0.0f, 0.0f, 8.0f),
        const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f),
        float yaw = -90.0f,
        float pitch = 0.0f);

    /**
     * @brief Returns the view matrix calculated from the camera's position and orientation.
     */
    glm::mat4 GetViewMatrix() const;

    /**
     * @brief Returns the current projection matrix (perspective).
     */
    glm::mat4 GetProjectionMatrix() const;

    /**
     * @brief Gets the current field-of-view (vertical FOV in degrees).
     */
    float GetFOV() const;

    /**
     * @brief Sets the field-of-view and updates the projection matrix accordingly.
     *
     * @param fov New FOV value in degrees.
     */
    void SetFOV(float fov);

    /**
     * @brief Moves the camera in the given direction according to its speed and deltaTime.
     *
     * @param direction  The desired CameraMovement direction (e.g. Forward).
     * @param deltaTime  Time elapsed (in seconds) since last frame.
     */
    void Move(CameraMovement direction, float deltaTime);

    /**
     * @brief Rotates the camera by xOffset (yaw) and yOffset (pitch).
     *
     * @param xOffset Change in yaw (degrees).
     * @param yOffset Change in pitch (degrees).
     */
    void Rotate(float xOffset, float yOffset);

    /**
     * @brief Sets the camera's movement speed (units per second).
     *
     * @param speed New speed value.
     */
    void SetSpeed(float speed);

    /**
     * @brief Dynamically computes a new FOV based on real-world display parameters.
     */
    void UpdateFOV();

    /**
     * @brief Recomputes the projection matrix with a given aspect ratio.
     *
     * @param aspectRatio Width/height ratio for the screen (>= 0).
     */
    void UpdateProjectionMatrix(float aspectRatio);

    /**
     * @brief Gets the camera's current position.
     */
    glm::vec3 GetPosition() const { return m_Position; }

    /**
     * @brief Directly accesses the camera's position by reference (use with caution).
     */
    glm::vec3& GetPositionRef() { return m_Position; }

    /**
     * @brief Gets the camera's forward (front) vector.
     */
    glm::vec3 GetFront() const { return m_Front; }

    /**
     * @brief Gets the camera's up vector.
     */
    glm::vec3 GetUp() const { return m_Up; }

    /**
     * @brief Gets the near clipping plane distance.
     */
    float GetNearPlane() const;

    /**
     * @brief Sets the near clipping plane and updates the projection matrix.
     *
     * @param nearPlane New near plane distance (minimum 0.01).
     */
    void SetNearPlane(float nearPlane);

    /**
     * @brief Gets the far clipping plane distance.
     */
    float GetFarPlane() const;

    /**
     * @brief Sets the far clipping plane and updates the projection matrix.
     *
     * @param farPlane New far plane distance (must be > near plane).
     */
    void SetFarPlane(float farPlane);

    /**
     * @brief Sets the camera's world-space position.
     *
     * @param pos The new position vector.
     */
    void SetPosition(const glm::vec3& pos) { m_Position = pos; }

private:
    /**
     * @brief Recalculates the front, right, and up vectors based on the current yaw and pitch.
     */
    void UpdateCameraVectors();

private:
    // -------------------
    // Camera Parameters
    // -------------------
    glm::vec3 m_Position;   ///< Current position in world space.
    glm::vec3 m_Front;      ///< Normalized forward direction vector.
    glm::vec3 m_Up;         ///< Normalized up direction (depends on front & right).
    glm::vec3 m_Right;      ///< Normalized right direction (cross of front & worldUp).
    glm::vec3 m_WorldUp;    ///< Keeps track of the "world up" direction.

    float m_Yaw;            ///< Rotation around the Y-axis (in degrees).
    float m_Pitch;          ///< Rotation around the X-axis (in degrees).
    float m_FOV;            ///< Vertical field of view (in degrees).
    float m_Speed;          ///< Movement speed in world units/second.
    float m_MouseSensitivity; ///< Mouse look sensitivity multiplier.
    float m_NearPlane;      ///< Near clipping plane distance.
    float m_FarPlane;       ///< Far clipping plane distance.

    // -------------------
    // Matrices
    // -------------------
    glm::mat4 m_ProjectionMatrix; ///< The computed perspective projection matrix.
};