#include "Scene/CameraController.h"
#include "Application/InputManager.h"
#include "Scene/FrameData.h"
#include <GLFW/glfw3.h>

CameraController::CameraController(Camera& camera, InputManager& inputManager)
    : m_Camera(camera),
    m_InputManager(inputManager),
    m_Sensitivity(0.1f),
    m_Speed(10.0f),
    m_LastX(0.0f),
    m_LastY(0.0f),
    m_FirstMouse(true)
{
}

void CameraController::Update(float deltaTime) {
    if (m_InputManager.IsKeyPressed(GLFW_KEY_UP))
        m_Camera.Move(CameraMovement::Forward, deltaTime);

    if (m_InputManager.IsKeyPressed(GLFW_KEY_DOWN))
        m_Camera.Move(CameraMovement::Backward, deltaTime);

    // Left movement - A key
    if (m_InputManager.IsKeyPressed(GLFW_KEY_A))
        m_Camera.Move(CameraMovement::Left, deltaTime);

    // Right movement - D key
    if (m_InputManager.IsKeyPressed(GLFW_KEY_D))
        m_Camera.Move(CameraMovement::Right, deltaTime);

    // Upward movement - Space key
    if (m_InputManager.IsKeyPressed(GLFW_KEY_W))
        m_Camera.Move(CameraMovement::Up, deltaTime);

    // Downward movement - Left Control key
    if (m_InputManager.IsKeyPressed(GLFW_KEY_S))
        m_Camera.Move(CameraMovement::Down, deltaTime);
}

void CameraController::ProcessMouseMovement(float xpos, float ypos) {
    if (m_FirstMouse) {
        m_LastX = xpos;
        m_LastY = ypos;
        m_FirstMouse = false;
        return;
    }

    float xOffset = xpos - m_LastX;
    float yOffset = m_LastY - ypos;

    m_LastX = xpos;
    m_LastY = ypos;

    m_Camera.Rotate(xOffset, yOffset);
}

void CameraController::ProcessMouseScroll(float yOffset) {
    float fov = m_Camera.GetFOV() - yOffset;
    m_Camera.SetFOV(fov);

    // Update projection matrix
    FrameData::s_Projection = glm::perspective(glm::radians(m_Camera.GetFOV()),
        static_cast<float>(Screen::s_Width) / static_cast<float>(Screen::s_Height),
        0.1f, 100.0f);
}

void CameraController::SetSpeed(float speed) {
    m_Speed = speed;
    m_Camera.SetSpeed(speed);
}

void CameraController::Reset() {
    m_FirstMouse = true;
}