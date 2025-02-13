#pragma once

#include "PostProcessingEffect.h"
#include <glm/glm.hpp>

class NoPostProcessingEffect : public PostProcessingEffect {
public:
    NoPostProcessingEffect(std::shared_ptr<graphics::MeshBuffer> quad, int width = 800, int height = 600);
    void Apply(GLuint inputTexture, GLuint outputFramebuffer) override;
    void OnWindowResize(int width, int height) override;
private:
};