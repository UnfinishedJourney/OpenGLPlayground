#include "Transform.h"
#include "gtc/matrix_transform.hpp"

glm::mat4 Transform::GetModelMatrix()
{
    if (b_NeedsUpdating)
        UpdateModelMatrix();

    return m_ModelMatrix;
}

void Transform::UpdateModelMatrix()
{
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), m_Position);
    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0), m_Rotation.w, glm::vec3(m_Rotation.x, m_Rotation.y, m_Rotation.z));
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), m_Scale);
    m_ModelMatrix = translationMatrix * rotationMatrix * scaleMatrix;

    b_NeedsUpdating = false;
}

