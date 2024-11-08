#pragma once

#include "Camera.h"
#include "Application/InputManager.h"

class CameraController {
public:
    CameraController(Camera& camera, InputManager& inputManager);

    void Update(float deltaTime);
    void ProcessMouseMovement(float xpos, float ypos);
    void ProcessMouseScroll(float yOffset);

    void SetSpeed(float speed);
    void Reset();

    // Methods to set display parameters dynamically
    void SetDisplayParameters(float displayHeight, float viewerDistance);

private:
    Camera& m_Camera;
    InputManager& m_InputManager;
    float m_Sensitivity;
    float m_Speed;
    float m_LastX, m_LastY;
    bool m_FirstMouse;
};