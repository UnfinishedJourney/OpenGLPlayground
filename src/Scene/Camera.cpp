#include "Camera.h"
#include "Screen.h" // Provides Screen::s_Width, s_Height, s_DisplayHeight, s_ViewerDistance
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

namespace Scene {

    Camera::Camera(const glm::vec3& position, const glm::vec3& up, float yaw, float pitch)
        : position_(position),
        worldUp_(up),
        yaw_(yaw),
        pitch_(pitch),
        fov_(45.0f),
        speed_(10.0f),
        mouseSensitivity_(0.1f),
        nearPlane_(0.01f),
        farPlane_(500.0f)
    {
        UpdateCameraVectors();

        float aspect = static_cast<float>(Screen::s_Width) / static_cast<float>(Screen::s_Height);
        projectionMatrix_ = glm::perspective(glm::radians(fov_), aspect, nearPlane_, farPlane_);
    }

    glm::mat4 Camera::GetViewMatrix() const {
        return glm::lookAt(position_, position_ + front_, up_);
    }

    glm::mat4 Camera::GetProjectionMatrix() const {
        return projectionMatrix_;
    }

    float Camera::GetFOV() const {
        return fov_;
    }

    void Camera::SetFOV(float fov) {
        fov_ = glm::clamp(fov, 1.0f, 120.0f);
        float aspect = static_cast<float>(Screen::s_Width) / static_cast<float>(Screen::s_Height);
        UpdateProjectionMatrix(aspect);
    }

    void Camera::Move(CameraMovement direction, float deltaTime) {
        float velocity = speed_ * deltaTime;
        switch (direction) {
        case CameraMovement::Forward:
            position_ += front_ * velocity;
            break;
        case CameraMovement::Backward:
            position_ -= front_ * velocity;
            break;
        case CameraMovement::Left:
            position_ -= right_ * velocity;
            break;
        case CameraMovement::Right:
            position_ += right_ * velocity;
            break;
        case CameraMovement::Up:
            position_ += worldUp_ * velocity;
            break;
        case CameraMovement::Down:
            position_ -= worldUp_ * velocity;
            break;
        }
    }

    void Camera::Rotate(float xOffset, float yOffset) {
        yaw_ += xOffset;
        pitch_ += yOffset;

        // Constrain pitch to avoid flip.
        pitch_ = glm::clamp(pitch_, -89.0f, 89.0f);
        UpdateCameraVectors();
    }

    void Camera::SetSpeed(float speed) {
        speed_ = speed;
    }

    float Camera::GetNearPlane() const {
        return nearPlane_;
    }

    void Camera::SetNearPlane(float nearPlane) {
        nearPlane_ = std::max(0.01f, nearPlane);
        float aspect = static_cast<float>(Screen::s_Width) / static_cast<float>(Screen::s_Height);
        UpdateProjectionMatrix(aspect);
    }

    float Camera::GetFarPlane() const {
        return farPlane_;
    }

    void Camera::SetFarPlane(float farPlane) {
        farPlane_ = std::max(farPlane, nearPlane_ + 0.1f);
        float aspect = static_cast<float>(Screen::s_Width) / static_cast<float>(Screen::s_Height);
        UpdateProjectionMatrix(aspect);
    }

    void Camera::UpdateFOV() {
        // Compute physical FOV based on real-world screen parameters.
        float displayHeight = Screen::s_DisplayHeight;   // in cm, for example
        float viewerDistance = Screen::s_ViewerDistance;   // in cm, for example
        float physicalFOV = 2.0f * glm::degrees(std::atan((displayHeight / 2.0f) / viewerDistance));
        fov_ = glm::clamp(physicalFOV, 1.0f, 120.0f);

        float aspect = static_cast<float>(Screen::s_Width) / static_cast<float>(Screen::s_Height);
        UpdateProjectionMatrix(aspect);
    }

    void Camera::UpdateProjectionMatrix(float aspectRatio) {
        projectionMatrix_ = glm::perspective(glm::radians(fov_), aspectRatio, nearPlane_, farPlane_);
    }

    void Camera::UpdateCameraVectors() {
        // Calculate the new Front vector from yaw and pitch.
        glm::vec3 newFront;
        newFront.x = std::cos(glm::radians(yaw_)) * std::cos(glm::radians(pitch_));
        newFront.y = std::sin(glm::radians(pitch_));
        newFront.z = std::sin(glm::radians(yaw_)) * std::cos(glm::radians(pitch_));
        front_ = glm::normalize(newFront);

        // Recalculate Right and Up vectors.
        right_ = glm::normalize(glm::cross(front_, worldUp_));
        up_ = glm::normalize(glm::cross(right_, front_));
    }

} // namespace Scene