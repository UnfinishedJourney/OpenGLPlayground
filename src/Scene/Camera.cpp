#include "Camera.h"
#include "Scene/Screen.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

Camera::Camera(const glm::vec3& position,
    const glm::vec3& up,
    float yaw,
    float pitch)
    : m_Position(position)
    , m_WorldUp(up)
    , m_Yaw(yaw)
    , m_Pitch(pitch)
    , m_FOV(45.0f)
    , m_Speed(10.0f)
    , m_MouseSensitivity(0.1f)
    , m_NearPlane(0.01f)
    , m_FarPlane(500.0f)
{
    UpdateCameraVectors();
    float aspect = static_cast<float>(Screen::s_Width) / static_cast<float>(Screen::s_Height);
    m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), aspect, m_NearPlane, m_FarPlane);
}

glm::mat4 Camera::GetViewMatrix() const
{
    return glm::lookAt(m_Position, m_Position + m_Front, m_Up);
}

glm::mat4 Camera::GetProjectionMatrix() const
{
    return m_ProjectionMatrix;
}

float Camera::GetFOV() const
{
    return m_FOV;
}

void Camera::SetFOV(float fov)
{
    m_FOV = glm::clamp(fov, 1.0f, 120.0f);
    UpdateProjectionMatrix(static_cast<float>(Screen::s_Width) / static_cast<float>(Screen::s_Height));
}

void Camera::Move(CameraMovement direction, float deltaTime)
{
    float velocity = m_Speed * deltaTime;
    switch (direction) {
    case CameraMovement::Forward:  m_Position += m_Front * velocity; break;
    case CameraMovement::Backward: m_Position -= m_Front * velocity; break;
    case CameraMovement::Left:     m_Position -= m_Right * velocity; break;
    case CameraMovement::Right:    m_Position += m_Right * velocity; break;
    case CameraMovement::Up:       m_Position += m_WorldUp * velocity; break;
    case CameraMovement::Down:     m_Position -= m_WorldUp * velocity; break;
    }
}

void Camera::Rotate(float xOffset, float yOffset)
{
    m_Yaw += xOffset;
    m_Pitch += yOffset;
    m_Pitch = glm::clamp(m_Pitch, -89.0f, 89.0f);
    UpdateCameraVectors();
}

void Camera::SetSpeed(float speed)
{
    m_Speed = speed;
}

float Camera::GetNearPlane() const
{
    return m_NearPlane;
}

void Camera::SetNearPlane(float nearPlane)
{
    m_NearPlane = std::max(0.01f, nearPlane);
    UpdateProjectionMatrix(static_cast<float>(Screen::s_Width) / static_cast<float>(Screen::s_Height));
}

float Camera::GetFarPlane() const
{
    return m_FarPlane;
}

void Camera::SetFarPlane(float farPlane)
{
    m_FarPlane = std::max(farPlane, m_NearPlane + 0.1f);
    UpdateProjectionMatrix(static_cast<float>(Screen::s_Width) / static_cast<float>(Screen::s_Height));
}

void Camera::UpdateFOV()
{
    float displayHeight = Screen::s_DisplayHeight;
    float viewerDistance = Screen::s_ViewerDistance;
    float physicalFOV = 2.0f * glm::degrees(std::atan((displayHeight / 2.0f) / viewerDistance));

    m_FOV = glm::clamp(physicalFOV, 1.0f, 120.0f);
    UpdateProjectionMatrix(static_cast<float>(Screen::s_Width) / static_cast<float>(Screen::s_Height));
}

void Camera::UpdateProjectionMatrix(float aspectRatio)
{
    m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), aspectRatio, m_NearPlane, m_FarPlane);
}

void Camera::UpdateCameraVectors()
{
    glm::vec3 front;
    front.x = std::cos(glm::radians(m_Yaw)) * std::cos(glm::radians(m_Pitch));
    front.y = std::sin(glm::radians(m_Pitch));
    front.z = std::sin(glm::radians(m_Yaw)) * std::cos(glm::radians(m_Pitch));
    m_Front = glm::normalize(front);

    m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
    m_Up = glm::normalize(glm::cross(m_Right, m_Front));
}