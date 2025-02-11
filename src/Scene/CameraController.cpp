#include "CameraController.h"
#include "Screen.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace Scene {

    CameraController::CameraController(InputManager& inputManager)
        : inputManager_(inputManager),
        sensitivity_(0.05f),
        speed_(10.0f),
        lastX_(0.0f),
        lastY_(0.0f),
        firstMouse_(true)
    {
    }

    void CameraController::Update(float deltaTime) {
        if (!camera_) return;

        // Keyboard-based movement.
        if (inputManager_.IsKeyPressed(GLFW_KEY_UP))
            camera_->Move(CameraMovement::Forward, deltaTime);
        if (inputManager_.IsKeyPressed(GLFW_KEY_DOWN))
            camera_->Move(CameraMovement::Backward, deltaTime);
        if (inputManager_.IsKeyPressed(GLFW_KEY_A))
            camera_->Move(CameraMovement::Left, deltaTime);
        if (inputManager_.IsKeyPressed(GLFW_KEY_D))
            camera_->Move(CameraMovement::Right, deltaTime);
        if (inputManager_.IsKeyPressed(GLFW_KEY_W))
            camera_->Move(CameraMovement::Up, deltaTime);
        if (inputManager_.IsKeyPressed(GLFW_KEY_S))
            camera_->Move(CameraMovement::Down, deltaTime);
    }

    void CameraController::ProcessMouseMovement(float xpos, float ypos) {
        if (!camera_) return;

        if (firstMouse_) {
            lastX_ = xpos;
            lastY_ = ypos;
            firstMouse_ = false;
            return;
        }

        float xOffset = xpos - lastX_;
        float yOffset = lastY_ - ypos; // Y is reversed
        lastX_ = xpos;
        lastY_ = ypos;

        xOffset *= sensitivity_;
        yOffset *= sensitivity_;

        camera_->Rotate(xOffset, yOffset);
    }

    void CameraController::ProcessMouseScroll(float yOffset) {
        if (!camera_) return;

        // Example: adjust FOV based on scroll input.
        float displayHeight = Screen::s_DisplayHeight;
        float viewerDistance = Screen::s_ViewerDistance;
        float physicalFOV = 2.0f * glm::degrees(std::atan((displayHeight / 2.0f) / viewerDistance));
        float fovChange = yOffset * 1.0f;
        float newFOV = camera_->GetFOV() - fovChange;

        float minFOV = physicalFOV - 10.0f;
        float maxFOV = physicalFOV + 10.0f;
        newFOV = glm::clamp(newFOV, minFOV, maxFOV);

        camera_->SetFOV(newFOV);
    }

    void CameraController::SetSpeed(float speed) {
        speed_ = speed;
        if (camera_) {
            camera_->SetSpeed(speed);
        }
    }

    void CameraController::Reset() {
        firstMouse_ = true;
    }

    void CameraController::UpdateFOV() {
        if (camera_) {
            camera_->UpdateFOV();
        }
    }

    void CameraController::SetCamera(const std::shared_ptr<Camera>& camera) {
        camera_ = camera;
    }

    bool CameraController::HasCamera() const {
        return (camera_ != nullptr);
    }

} // namespace Scene