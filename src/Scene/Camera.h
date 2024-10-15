#pragma once
#include "Scene/FrameData.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/constants.hpp"

enum CameraMovement {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

class Camera
{
public:
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f),
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
        float yaw = -90.0f, float pitch = 0.0f, float fov = 45.0f)
        : m_CameraPos(position), m_CameraUp(up), m_Yaw(yaw), m_Pitch(pitch), m_FOV(fov), m_CameraSpeed(10.0f)
    {
        UpdateCameraVectors();
    }

    glm::mat4 GetViewMatrix() const {
        return glm::lookAt(m_CameraPos, m_CameraPos + m_CameraFront, m_CameraUp);
    }

    float GetFOV() const {
        return m_FOV;
    }

    void SetFOV(float fov) {
        m_FOV = fov;
    }

    void Move(CameraMovement direction, float deltaTime) {
        float velocity = m_CameraSpeed * deltaTime;
        if (direction == UP)
            m_CameraPos += m_CameraUp * velocity;
        if (direction == DOWN)
            m_CameraPos -= m_CameraUp * velocity;
        if (direction == LEFT)
            m_CameraPos -= glm::normalize(glm::cross(m_CameraFront, m_CameraUp)) * velocity;
        if (direction == RIGHT)
            m_CameraPos += glm::normalize(glm::cross(m_CameraFront, m_CameraUp)) * velocity;
    }

    void Rotate(float xOffset, float yOffset) {
        m_Yaw += xOffset;
        m_Pitch += yOffset;

        if (m_Pitch > 89.0f)
            m_Pitch = 89.0f;
        if (m_Pitch < -89.0f)
            m_Pitch = -89.0f;

        UpdateCameraVectors();
    }

    void SetSpeed(float speed) {
        m_CameraSpeed = speed;
    }

    glm::vec3 GetFront() const {
        return m_CameraFront;
    }

    glm::vec3 GetUp() const {
        return m_CameraUp;
    }

private:
    glm::vec3 m_CameraPos;
    glm::vec3 m_CameraFront;
    glm::vec3 m_CameraUp;
    glm::vec3 m_CameraRight;

    float m_Yaw;
    float m_Pitch;
    float m_FOV;
    float m_CameraSpeed;

    void UpdateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        front.y = sin(glm::radians(m_Pitch));
        front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        m_CameraFront = glm::normalize(front);
        m_CameraRight = glm::normalize(glm::cross(m_CameraFront, glm::vec3(0.0f, 1.0f, 0.0f)));
        m_CameraUp = glm::normalize(glm::cross(m_CameraRight, m_CameraFront));
    }
};

class CameraController {
public:
    CameraController(Camera& camera, float sensitivity = 0.1f, float speed = 10.0f)
        : m_Camera(camera), m_Sensitivity(sensitivity), m_Speed(speed), m_LastX(0.0f), m_LastY(0.0f), m_FirstMouse(true)
    {}

    void ProcessKeyboard(CameraMovement direction, float deltaTime) {
        m_Camera.Move(direction, deltaTime);
    }

    void ProcessMouseMovement(float xpos, float ypos) {
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

        xOffset *= m_Sensitivity;
        yOffset *= m_Sensitivity;

        m_Camera.Rotate(xOffset, yOffset);
    }

    void ProcessMouseScroll(float yOffset) {
        float fov = m_Camera.GetFOV() - yOffset;
        fov = glm::clamp(fov, 1.0f, 120.0f);
        m_Camera.SetFOV(fov);
        FrameData::s_Projection = glm::perspective(glm::radians(fov), (float)Screen::s_Width / (float)Screen::s_Height, 0.1f, 100.0f);
    }

    void SetSpeed(float speed) {
        m_Speed = speed;
        m_Camera.SetSpeed(speed);
    }

    void Reset() {
        m_FirstMouse = true;
    }

private:
    Camera& m_Camera;
    float m_Sensitivity;
    float m_Speed;
    float m_LastX, m_LastY;
    bool m_FirstMouse;
};