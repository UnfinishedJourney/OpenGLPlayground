#pragma once

#include "glm.hpp"
#include "gtc/constants.hpp"
#include "gtc/matrix_transform.hpp"

class MyCamera
{
public:
	//MyCamera();
	//~MyCamera();
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

private:
	glm::vec3 m_CameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 m_CameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 m_CameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	float m_FOV;
};
