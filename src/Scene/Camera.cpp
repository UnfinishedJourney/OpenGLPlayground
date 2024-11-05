#include "Scene/Camera.h"
#include "Scene/FrameData.h"

Camera::Camera(const glm::vec3& position, const glm::vec3& up, float yaw, float pitch, float fov)
    : m_Position(position),
    m_WorldUp(up),
    m_Yaw(yaw),
    m_Pitch(pitch),
    m_FOV(fov),
    m_Speed(10.0f),
    m_MouseSensitivity(0.1f)
{
    UpdateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(m_Position, m_Position + m_Front, m_Up);
}

float Camera::GetFOV() const {
    return m_FOV;
}

void Camera::SetFOV(float fov) {
    m_FOV = glm::clamp(fov, 1.0f, 120.0f);
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

    FrameData::s_CameraPos = m_Position;
}

void Camera::Rotate(float xOffset, float yOffset) {
    xOffset *= m_MouseSensitivity;
    yOffset *= m_MouseSensitivity;

    m_Yaw += xOffset;
    m_Pitch += yOffset;

    // Constrain the pitch
    if (m_Pitch > 89.0f)
        m_Pitch = 89.0f;
    if (m_Pitch < -89.0f)
        m_Pitch = -89.0f;

    UpdateCameraVectors();
}

void Camera::SetSpeed(float speed) {
    m_Speed = speed;
}

void Camera::UpdateCameraVectors() {
    // Calculate the new Front vector
    glm::vec3 front;
    front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    front.y = sin(glm::radians(m_Pitch));
    front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    m_Front = glm::normalize(front);

    // Also re-calculate the Right and Up vector
    m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));  // Normalize the vectors
    m_Up = glm::normalize(glm::cross(m_Right, m_Front));

    // Update the FrameData view matrix
    FrameData::s_View = GetViewMatrix();
    FrameData::s_CameraPos = m_Position;
}