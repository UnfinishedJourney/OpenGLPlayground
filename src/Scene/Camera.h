#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

enum class CameraMovement {
    Forward,
    Backward,
    Left,
    Right,
    Up,
    Down
};

class Camera
{
public:
    Camera(const glm::vec3& position = glm::vec3(0.0f, 0.0f, 8.0f),
        const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f),
        float yaw = -90.0f, float pitch = 0.0f);

    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix() const;

    float GetFOV() const;
    void SetFOV(float fov);

    void Move(CameraMovement direction, float deltaTime);
    void Rotate(float xOffset, float yOffset);
    void SetSpeed(float speed);

    void UpdateFOV();
    void UpdateProjectionMatrix(float aspectRatio);

    glm::vec3 GetPosition() const { return m_Position; }
    glm::vec3 GetFront() const { return m_Front; }
    glm::vec3 GetUp() const { return m_Up; }

    float GetNearPlane() const;

    void SetNearPlane(float nearPlane);

    float GetFarPlane() const;

    void SetFarPlane(float farPlane);

private:
    void UpdateCameraVectors();

    glm::vec3 m_Position;
    glm::vec3 m_Front;
    glm::vec3 m_Up;
    glm::vec3 m_Right;
    glm::vec3 m_WorldUp;

    float m_Yaw;
    float m_Pitch;
    float m_FOV;
    float m_Speed;
    float m_MouseSensitivity;
    float m_NearPlane;
    float m_FarPlane;
    glm::mat4 m_ProjectionMatrix;
};