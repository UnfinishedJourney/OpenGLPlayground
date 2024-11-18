#pragma once

#include "PostProcessingEffect.h"
#include <glm/glm.hpp>

class EdgeDetectionEffect : public PostProcessingEffect {
public:
    EdgeDetectionEffect();
    void Initialize() override;
    void Apply(GLuint inputTexture, GLuint outputFramebuffer) override;
    void OnWindowResize(int width, int height) override;

    void SetEdgeThreshold(float threshold);

private:
    float m_EdgeThreshold;
    int m_Width, m_Height;
    glm::vec2 m_TexelSize;
};