#pragma once

#include "PostProcessingEffect.h"
#include <glm/glm.hpp>

class EdgeDetectionEffect : public PostProcessingEffect {
public:
    EdgeDetectionEffect(std::shared_ptr<MeshBuffer> quad, int width = 800, int height = 600);
    void Apply(GLuint inputTexture, GLuint outputFramebuffer) override;
    void OnWindowResize(int width, int height) override;

    void SetEdgeThreshold(float threshold);
    void SetParameters(const std::unordered_map<std::string, float>& params) override;
private:
    float m_EdgeThreshold;
    glm::vec2 m_TexelSize;
};