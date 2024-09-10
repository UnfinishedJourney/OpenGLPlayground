#pragma once

#include "glm.hpp"
#include "gtc/constants.hpp"
#include "gtc/matrix_transform.hpp"

enum CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

class Camera
{
public:
    Camera()
    {}

    ~Camera()
    {}

    void ProcessKeyboard(CameraMovement direction, float deltaTime)
    {
        float velocity = m_CameraSpeed * deltaTime;
        if (direction == FORWARD)
            m_CameraPos += m_CameraUp*velocity;
        if (direction == BACKWARD)
            m_CameraPos -= m_CameraUp*velocity;
        if (direction == LEFT)
            m_CameraPos -= glm::normalize(glm::cross(m_CameraFront, m_CameraUp)) * velocity;
        if (direction == RIGHT)
            m_CameraPos += glm::normalize(glm::cross(m_CameraFront, m_CameraUp)) * velocity;
    }

	void MovePos(glm::vec3 deltaCameraPos) {
		m_CameraPos += deltaCameraPos;
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

private:
	glm::vec3 m_CameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 m_CameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 m_CameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	float m_FOV = 45.0f;
    float m_CameraSpeed = 10.0f;
};