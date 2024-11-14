#include "CameraController.h"
#include "Screen.h"
#include <GLFW/glfw3.h>
#include <algorithm> // For std::clamp

CameraController::CameraController(InputManager& inputManager)
    :   m_InputManager(inputManager),
        m_Sensitivity(0.05f), 
        m_Speed(10.0f),
        m_LastX(0.0f),
        m_LastY(0.0f),
        m_FirstMouse(true)
{
}

void CameraController::Update(float deltaTime) {
    if (!m_Camera) 
        return;

    if (m_InputManager.IsKeyPressed(GLFW_KEY_UP))
        m_Camera->Move(CameraMovement::Forward, deltaTime);

    if (m_InputManager.IsKeyPressed(GLFW_KEY_DOWN))
        m_Camera->Move(CameraMovement::Backward, deltaTime);

    if (m_InputManager.IsKeyPressed(GLFW_KEY_A))
        m_Camera->Move(CameraMovement::Left, deltaTime);

    if (m_InputManager.IsKeyPressed(GLFW_KEY_D))
        m_Camera->Move(CameraMovement::Right, deltaTime);

    if (m_InputManager.IsKeyPressed(GLFW_KEY_W))
        m_Camera->Move(CameraMovement::Up, deltaTime);

    if (m_InputManager.IsKeyPressed(GLFW_KEY_S))
        m_Camera->Move(CameraMovement::Down, deltaTime);
}

void CameraController::ProcessMouseMovement(float xpos, float ypos) {
    if (!m_Camera) 
        return;

    if (m_FirstMouse) {
        m_LastX = xpos;
        m_LastY = ypos;
        m_FirstMouse = false;
        return;
    }

    float xOffset = xpos - m_LastX;
    float yOffset = m_LastY - ypos; // Reversed since y-coordinates go from bottom to top

    m_LastX = xpos;
    m_LastY = ypos;

    m_Camera->Rotate(xOffset, yOffset);
}

void CameraController::ProcessMouseScroll(float yOffset) {
    if (!m_Camera)
        return;
    float displayHeight = Screen::s_DisplayHeight;
    float viewerDistance = Screen::s_ViewerDistance;
    float physicalFOV = 2.0f * glm::degrees(std::atan((displayHeight / 2.0f) / viewerDistance));

    // Apply scroll-based FOV adjustments within constraints
    float fovChange = yOffset * 1.0f; // Adjust the multiplier for desired sensitivity
    float newFOV = m_Camera->GetFOV() - fovChange;

    // Clamp the new FOV to within physicalFOV � 10 degrees
    newFOV = glm::clamp(newFOV, physicalFOV - 10.0f, physicalFOV + 10.0f);

    m_Camera->SetFOV(newFOV);
}

void CameraController::SetSpeed(float speed) {
    if (!m_Camera)
        return;
    m_Speed = speed;
    m_Camera->SetSpeed(speed);
}

void CameraController::Reset() {
    m_FirstMouse = true;
}

//need to add send to renderer fbo
void CameraController::UpdateFOV() {
    if (!m_Camera)
        return;

    m_Camera->UpdateFOV();
}