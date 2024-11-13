#include "Camera.h"
#include "Screen.h"
#include <algorithm> // For std::clamp
#include <cmath>     // For atan

Camera::Camera(
    const glm::vec3& position,
    const glm::vec3& up,
    float yaw,
    float pitch
)
    : m_Position(position),
    m_WorldUp(up),
    m_Yaw(yaw),
    m_Pitch(pitch),
    m_Speed(10.0f),
    m_MouseSensitivity(0.1f),
    m_FOV(45.0f)
{
    UpdateCameraVectors();

    // Initialize the projection matrix
    float aspectRatio = static_cast<float>(Screen::s_Width) / static_cast<float>(Screen::s_Height);
    m_ProjectionMatrix = glm::perspective(
        glm::radians(m_FOV),
        aspectRatio,
        0.1f,
        500.0f
    );
}

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(m_Position, m_Position + m_Front, m_Up);
}

glm::mat4 Camera::GetProjectionMatrix() const {
    return m_ProjectionMatrix;
}

float Camera::GetFOV() const {
    return m_FOV;
}

void Camera::SetFOV(float fov) {
    // Clamp the FOV to prevent extreme distortion
    m_FOV = glm::clamp(fov, 1.0f, 120.0f);
    UpdateProjectionMatrix(static_cast<float>(Screen::s_Width) / static_cast<float>(Screen::s_Height));
}

void Camera::Move(CameraMovement direction, float deltaTime) {
    float velocity = m_Speed * deltaTime;
    if (direction == CameraMovement::Forward)
        m_Position += m_Front * velocity;
    if (direction == CameraMovement::Backward)
        m_Position -= m_Front * velocity;
    if (direction == CameraMovement::Left)
        m_Position -= m_Right * velocity;
    if (direction == CameraMovement::Right)
        m_Position += m_Right * velocity;
    if (direction == CameraMovement::Up)
        m_Position += m_WorldUp * velocity;
    if (direction == CameraMovement::Down)
        m_Position -= m_WorldUp * velocity;
}

void Camera::Rotate(float xOffset, float yOffset) {
    xOffset *= m_MouseSensitivity;
    yOffset *= m_MouseSensitivity;

    m_Yaw += xOffset;
    m_Pitch += yOffset;

    // Constrain the pitch to prevent flipping
    m_Pitch = glm::clamp(m_Pitch, -89.0f, 89.0f);

    UpdateCameraVectors();
}

void Camera::SetSpeed(float speed) {
    m_Speed = speed;
}

void Camera::UpdateFOV()
{
    // Access static members from Screen class
    float displayHeight = Screen::s_DisplayHeight;
    float viewerDistance = Screen::s_ViewerDistance;

    // Calculate the physical FOV based on display size and viewer distance
    float physicalFOV = 2.0f * glm::degrees(std::atan((displayHeight / 2.0f) / viewerDistance));

    // Clamp the FOV to prevent extreme distortion
    m_FOV = glm::clamp(physicalFOV, 1.0f, 120.0f);

    UpdateProjectionMatrix(static_cast<float>(Screen::s_Width) / static_cast<float>(Screen::s_Height));
}

void Camera::UpdateProjectionMatrix(float aspectRatio) {
    m_ProjectionMatrix = glm::perspective(
        glm::radians(m_FOV),
        aspectRatio,
        0.1f,
        500.0f
    );
}

void Camera::UpdateCameraVectors() {
    // Calculate the new Front vector
    glm::vec3 front;
    front.x = std::cos(glm::radians(m_Yaw)) * std::cos(glm::radians(m_Pitch));
    front.y = std::sin(glm::radians(m_Pitch));
    front.z = std::sin(glm::radians(m_Yaw)) * std::cos(glm::radians(m_Pitch));
    m_Front = glm::normalize(front);

    // Re-calculate the Right and Up vectors
    m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));  // Normalize the vectors
    m_Up = glm::normalize(glm::cross(m_Right, m_Front));
}