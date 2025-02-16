#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

namespace Scene {

    enum class CameraMovement {
        Forward,
        Backward,
        Left,
        Right,
        Up,
        Down
    };

    class Camera {
    public:
        explicit Camera(const glm::vec3& position = glm::vec3(0.0f, 0.0f, 8.0f),
            const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f),
            float yaw = -90.0f,
            float pitch = 0.0f);

        glm::mat4 GetViewMatrix() const;
        glm::mat4 GetProjectionMatrix() const;
        float GetFOV() const;
        void SetFOV(float fov);

        void Move(CameraMovement direction, float deltaTime);
        void Rotate(float xOffset, float yOffset);
        void SetSpeed(float speed);

        void UpdateFOV();
        void UpdateProjectionMatrix(float aspectRatio);

        glm::vec3 GetPosition() const { return position_; }
        glm::vec3& GetPositionRef() { return position_; }
        glm::vec3 GetFront() const { return front_; }
        glm::vec3 GetUp() const { return up_; }

        float GetNearPlane() const;
        void SetNearPlane(float nearPlane);
        float GetFarPlane() const;
        void SetFarPlane(float farPlane);

        void SetPosition(const glm::vec3& pos) { position_ = pos; }

    private:
        void UpdateCameraVectors();

    private:
        glm::vec3 position_;
        glm::vec3 front_;
        glm::vec3 up_;
        glm::vec3 right_;
        glm::vec3 worldUp_;

        float yaw_;
        float pitch_;
        float fov_;
        float speed_;
        float mouseSensitivity_;
        float nearPlane_;
        float farPlane_;

        glm::mat4 projectionMatrix_;
    };

} // namespace Scene