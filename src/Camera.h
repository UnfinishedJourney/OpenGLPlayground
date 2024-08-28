#pragma once

#include "glm.hpp"
#include "gtc/constants.hpp"
#include "gtc/matrix_transform.hpp"

class MyCamera
{
public:
    MyCamera()
    {

    }

    ~MyCamera()
    {

    }
	void MovePos(glm::vec3 deltaCameraPos)
	{
		m_CameraPos += deltaCameraPos;
	}

	glm::mat4 GetViewMatrix() const
	{
		return glm::lookAt(m_CameraPos, m_CameraFront, m_CameraUp);
	}

	float GetFOV() const
	{
		return m_FOV;
	}

    void SetFOV(float fov)
    {
        m_FOV = fov;
    }

private:
	glm::vec3 m_CameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 m_CameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 m_CameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	float m_FOV = 45.0f;
};

class Camera
{
public:
    Camera() : m_Yaw(-90.0f), m_Pitch(0.0f), m_FOV(45.0f) {}

    void MovePos(glm::vec3 deltaCameraPos)
    {
        m_CameraPos += deltaCameraPos;
    }

    glm::mat4 GetViewMatrix() const
    {
        return glm::lookAt(m_CameraPos, m_CameraPos + m_CameraFront, m_CameraUp);
    }

    float GetFOV() const
    {
        return m_FOV;
    }

    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true)
    {
        float sensitivity = 0.1f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        m_Yaw += xoffset;
        m_Pitch += yoffset;

        if (constrainPitch)
        {
            if (m_Pitch > 89.0f)
                m_Pitch = 89.0f;
            if (m_Pitch < -89.0f)
                m_Pitch = -89.0f;
        }

        UpdateCameraVectors();
    }

    void ProcessMouseScroll(float yoffset)
    {
        m_FOV -= yoffset;
        if (m_FOV < 1.0f)
            m_FOV = 1.0f;
        if (m_FOV > 45.0f)
            m_FOV = 45.0f;
    }

    void ProcessKeyboard(int direction, float deltaTime)
    {
        float velocity = m_MovementSpeed * deltaTime;
        if (direction == 0) // W
            m_CameraPos += m_CameraFront * velocity;
        if (direction == 1) // S
            m_CameraPos -= m_CameraFront * velocity;
        if (direction == 2) // A
            m_CameraPos -= glm::normalize(glm::cross(m_CameraFront, m_CameraUp)) * velocity;
        if (direction == 3) // D
            m_CameraPos += glm::normalize(glm::cross(m_CameraFront, m_CameraUp)) * velocity;
    }

private:
    glm::vec3 m_CameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 m_CameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 m_CameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 m_CameraRight;
    glm::vec3 m_CameraWorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    float m_Yaw;
    float m_Pitch;
    float m_FOV;
    float m_MovementSpeed = 2.5f;

    void UpdateCameraVectors()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        front.y = sin(glm::radians(m_Pitch));
        front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        m_CameraFront = glm::normalize(front);
        m_CameraRight = glm::normalize(glm::cross(m_CameraFront, m_CameraWorldUp));
        m_CameraUp = glm::normalize(glm::cross(m_CameraRight, m_CameraFront));
    }
};