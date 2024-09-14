#pragma once

#include "glm.hpp"

class Transform {
public:
    Transform()
        : m_Position(0.0f), m_Rotation(0.0f), m_Scale(1.0f), m_ModelMatrix(1.0)
    {}

    void SetPosition(glm::vec3 position)
    {
        m_Position = position;

        b_NeedsUpdating = true;
    }

    void SetRotation(glm::vec4 rotation)
    {
        m_Rotation = rotation;

        b_NeedsUpdating = true;
    }

    void SetScale(glm::vec3 scale)
    {
        m_Scale = scale;

        b_NeedsUpdating = true;
    }

    void AddPosition(glm::vec3 deltaPos)
    {
        m_Position += deltaPos;

        b_NeedsUpdating = true;
    }

    void AddRotation(float deltaAngle)
    {
        m_Rotation.w += deltaAngle;
        
        b_NeedsUpdating = true;
    }
    
    void MultiplyScale(glm::vec3 scaleMult)
    {
        m_Scale.x *= scaleMult.x;
        m_Scale.y *= scaleMult.y;
        m_Scale.z *= scaleMult.z;

        b_NeedsUpdating = true;
    }

    glm::mat4 GetModelMatrix();

private:
    glm::vec3 m_Position;
    glm::vec4 m_Rotation; //axis, angle
    glm::vec3 m_Scale;
    glm::mat4 m_ModelMatrix;

    bool b_NeedsUpdating = false;
    void UpdateModelMatrix();
};