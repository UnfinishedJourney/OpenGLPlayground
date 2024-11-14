#pragma once

#include "glm/glm.hpp"

class Light {
public:
    enum class Type {
        Directional,
        Point,
    };

    Light();
    Light(Type type, const glm::vec3& position, const glm::vec3& color, float intensity = 1.0f);

    void SetType(Type type);
    void SetPosition(const glm::vec3& position);
    void SetColor(const glm::vec3& color);
    void SetIntensity(float intensity);

    Type GetType() const;
    const glm::vec3& GetPosition() const;
    //const glm::vec3& GetDirection() const;
    const glm::vec3& GetColor() const;
    float GetIntensity() const;

private:
    Type m_Type;
    glm::vec3 m_Position;
    glm::vec3 m_Color;
    float m_Intensity;
};

struct alignas(16) LightData {
    glm::vec4 position; // w is padding or another parameter, for dir lights position is direction? don't have them yet
    glm::vec4 color;    // w is intensity
};