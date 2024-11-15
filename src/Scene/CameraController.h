#pragma once

#include "Camera.h"
#include "Application/InputManager.h"
#include <memory>

class CameraController {
public:
    CameraController(InputManager& inputManager);

    void Update(float deltaTime);
    void ProcessMouseMovement(float xpos, float ypos);
    void ProcessMouseScroll(float yOffset);

    void SetSpeed(float speed);
    void Reset();

    void UpdateFOV();
    void SetCamera(const std::shared_ptr<Camera>& camera) { m_Camera = camera; }
    bool HasCamera() const { return m_Camera != nullptr; }

private:
    std::shared_ptr<Camera> m_Camera; 
    InputManager& m_InputManager;
    float m_Sensitivity;
    float m_Speed;
    float m_LastX, m_LastY;
    bool m_FirstMouse;
};