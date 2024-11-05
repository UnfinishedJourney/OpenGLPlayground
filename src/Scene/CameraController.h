#pragma once

#include "Scene/Camera.h"
#include "Application/InputManager.h"

class CameraController {
public:
    CameraController(Camera& camera, InputManager& inputManager);

    void Update(float deltaTime);
    void ProcessMouseMovement(float xpos, float ypos);
    void ProcessMouseScroll(float yOffset);

    void SetSpeed(float speed);
    void Reset();

private:
    Camera& m_Camera;
    InputManager& m_InputManager;
    float m_Sensitivity;
    float m_Speed;
    float m_LastX, m_LastY;
    bool m_FirstMouse;
};