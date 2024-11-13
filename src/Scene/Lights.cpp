#include "Lights.h"

Light::Light()
    : m_Type(Type::Point),
    m_Position(0.0f),
    m_Color(1.0f),
    m_Intensity(1.0f)
{
}

Light::Light(Type type, const glm::vec3& position, const glm::vec3& color, float intensity)
    : m_Type(type),
    m_Position(position),
    m_Color(color),
    m_Intensity(intensity)
{
}

void Light::SetType(Type type) {
    m_Type = type;
}

void Light::SetPosition(const glm::vec3& position) {
    m_Position = position;
}


void Light::SetColor(const glm::vec3& color) {
    m_Color = color;
}

void Light::SetIntensity(float intensity) {
    m_Intensity = intensity;
}

Light::Type Light::GetType() const {
    return m_Type;
}

const glm::vec3& Light::GetPosition() const {
    return m_Position;
}


const glm::vec3& Light::GetColor() const {
    return m_Color;
}

float Light::GetIntensity() const {
    return m_Intensity;
}