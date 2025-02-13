#pragma once

#include "PostProcessingEffect.h"
#include <glm/glm.hpp>

class PresentTextureEffect : public PostProcessingEffect {
public:
    PresentTextureEffect(std::shared_ptr<graphics::MeshBuffer> quad, int width = 800, int height = 600);
    void Apply(GLuint inputTexture, GLuint outputFramebuffer) override;
    void OnWindowResize(int width, int height) override;
    void SetParameters(const std::unordered_map<std::string, EffectParameter>& params) override;
private:
    std::shared_ptr<graphics::ITexture> m_Texture;
    void SetTexture(std::shared_ptr<graphics::ITexture> texture);
};