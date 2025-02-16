#pragma once

#include <memory>
#include <string>
#include "Application/InputManager.h"
#include "Camera.h"

namespace Scene {

    class CameraController {
    public:
        explicit CameraController(InputManager& inputManager);

        void Update(float deltaTime);
        void ProcessMouseMovement(float xpos, float ypos);
        void ProcessMouseScroll(float yOffset);
        void SetSpeed(float speed);
        void Reset();
        void UpdateFOV();

        void SetCamera(const std::shared_ptr<Camera>& camera);
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