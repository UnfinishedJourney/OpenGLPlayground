#include "CameraController.h"
#include "FrameData.h"
#include <GLFW/glfw3.h>
#include <algorithm> // For std::clamp

CameraController::CameraController(Camera& camera, InputManager& inputManager)
    : m_Camera(camera),
    m_InputManager(inputManager),
    m_Sensitivity(0.05f), // Reduced sensitivity for smoother rotation
    m_Speed(10.0f),
    m_LastX(0.0f),
    m_LastY(0.0f),
    m_FirstMouse(true)
{
    // Camera already updates FOV based on Screen's static parameters
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
    float yOffset = m_LastY - ypos; // Reversed since y-coordinates go from bottom to top

    m_LastX = xpos;
    m_LastY = ypos;

    m_Camera.Rotate(xOffset, yOffset);
}

void CameraController::ProcessMouseScroll(float yOffset) {
    // Calculate the physical FOV based on current Screen's static parameters
    float displayHeight = Screen::s_DisplayHeight;
    float viewerDistance = Screen::s_ViewerDistance;
    float physicalFOV = 2.0f * glm::degrees(std::atan((displayHeight / 2.0f) / viewerDistance));

    // Apply scroll-based FOV adjustments within constraints
    float fovChange = yOffset * 1.0f; // Adjust the multiplier for desired sensitivity
    float newFOV = m_Camera.GetFOV() - fovChange;

    // Clamp the new FOV to within physicalFOV ± 10 degrees
    newFOV = glm::clamp(newFOV, physicalFOV - 10.0f, physicalFOV + 10.0f);

    m_Camera.SetFOV(newFOV);
}

void CameraController::SetSpeed(float speed) {
    m_Speed = speed;
    m_Camera.SetSpeed(speed);
}

void CameraController::Reset() {
    m_FirstMouse = true;
}

void CameraController::SetDisplayParameters(float displayHeight, float viewerDistance) {

    // For example, if you allow changing the viewer distance at runtime:
    Screen::s_DisplayHeight = displayHeight;
    Screen::s_ViewerDistance = viewerDistance;

    m_Camera.UpdateFOV();
}