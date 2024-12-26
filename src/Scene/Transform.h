#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include <algorithm> 

class Transform {
public:
    Transform()
        : m_ModelMatrix(1.0f), m_NormalMatrix(1.0)
    {}

    glm::mat4 GetModelMatrix() {
        return m_ModelMatrix;
    }

    glm::mat3 GetNormalMatrix() {
        return m_NormalMatrix;
    }

    void SetModelMatrix(const glm::mat4& matrix)
    {
        m_ModelMatrix = matrix;
        m_NormalMatrix = glm::transpose(glm::inverse(glm::mat3(matrix)));
    }

private:
    glm::mat3 m_NormalMatrix;
    glm::mat4 m_ModelMatrix;
};