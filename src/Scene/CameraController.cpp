#include "CameraController.h"
#include "Scene/Screen.h"
#include <GLFW/glfw3.h>

CameraController::CameraController(InputManager& inputManager)
    : m_InputManager(inputManager)
    , m_Sensitivity(0.05f)
    , m_Speed(10.0f)
    , m_LastX(0.0f)
    , m_LastY(0.0f)
    , m_FirstMouse(true)
{
}

void CameraController::Update(float deltaTime)
{
    if (!m_Camera) return;

    // Keyboard-based movement
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

void CameraController::ProcessMouseMovement(float xpos, float ypos)
{
    if (!m_Camera) return;

    if (m_FirstMouse)
    {
        // Initialize the last mouse positions so the first movement isn't huge
        m_LastX = xpos;
        m_LastY = ypos;
        m_FirstMouse = false;
        return;
    }

    float xOffset = xpos - m_LastX;
    float yOffset = m_LastY - ypos;  // Y is reversed (top vs bottom)
    m_LastX = xpos;
    m_LastY = ypos;

    // Scale by sensitivity
    xOffset *= m_Sensitivity;
    yOffset *= m_Sensitivity;

    // Rotate the camera
    m_Camera->Rotate(xOffset, yOffset);
}

void CameraController::ProcessMouseScroll(float yOffset)
{
    if (!m_Camera) return;

    // Example approach: clamp around a "physical" FOV ± 10 degrees
    float displayHeight = Screen::s_DisplayHeight;
    float viewerDistance = Screen::s_ViewerDistance;

    float physicalFOV = 2.0f * glm::degrees(std::atan((displayHeight / 2.0f) / viewerDistance));
    float fovChange = yOffset * 1.0f;
    float newFOV = m_Camera->GetFOV() - fovChange;

    // Clamp to (physicalFOV - 10, physicalFOV + 10)
    float minFOV = physicalFOV - 10.0f;
    float maxFOV = physicalFOV + 10.0f;
    newFOV = glm::clamp(newFOV, minFOV, maxFOV);

    m_Camera->SetFOV(newFOV);
}

void CameraController::SetSpeed(float speed)
{
    m_Speed = speed;
    if (m_Camera)
    {
        m_Camera->SetSpeed(speed);
    }
}

void CameraController::Reset()
{
    // Next time we move the mouse, treat it as the first event
    m_FirstMouse = true;
}

void CameraController::UpdateFOV()
{
    if (m_Camera)
    {
        m_Camera->UpdateFOV();
    }
}