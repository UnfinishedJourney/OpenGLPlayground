#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

class Transform {
public:
    Transform()
        : m_Position(glm::vec3(0.0, 0.0, 0.0)), m_Rotation(glm::quat(glm::vec3(0.0, 0.0, 0.0))), m_Scale(1.0f), m_ModelMatrix(1.0f), m_NormalMatrix(1.0), m_NeedsUpdating(true)
    {}

    void SetPosition(const glm::vec3& position) {
        m_Position = position;
        m_NeedsUpdating = true;
    }

    glm::quat GetRotation() {
        return m_Rotation;
    }

    void SetRotation(const glm::vec3& eulerAngles) {
        m_Rotation = glm::quat(eulerAngles);
        m_NeedsUpdating = true;
    }

    void SetScale(const glm::vec3& scale) {
        m_Scale = scale;
        m_NeedsUpdating = true;
    }

    void AddPosition(const glm::vec3& deltaPos) {
        m_Position += deltaPos;
        m_NeedsUpdating = true;
    }

    void AddRotation(const glm::vec3& deltaEulerAngles) {
        glm::quat deltaRot = glm::quat(deltaEulerAngles);
        m_Rotation = glm::normalize(deltaRot * m_Rotation);
        m_NeedsUpdating = true;
    }

    glm::mat4 GetModelMatrix() {
        if (m_NeedsUpdating) {
            UpdateModelMatrix();
        }
        return m_ModelMatrix;
    }

    glm::mat3 GetNormalMatrix() {
        if (m_NeedsUpdating) {
            UpdateModelMatrix();
        }
        return m_NormalMatrix;
    }

private:
    glm::vec3 m_Position;
    glm::quat m_Rotation;
    glm::vec3 m_Scale;
    glm::mat3 m_NormalMatrix;
    glm::mat4 m_ModelMatrix;
    bool m_NeedsUpdating;

    void UpdateModelMatrix() {
        m_ModelMatrix = glm::translate(glm::mat4(1.0f), m_Position) *
            glm::mat4_cast(m_Rotation) *
            glm::scale(glm::mat4(1.0f), m_Scale);
        m_NormalMatrix = glm::transpose(glm::inverse(glm::mat3(m_ModelMatrix)));
        m_NeedsUpdating = false;
    }
};