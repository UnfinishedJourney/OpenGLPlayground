#pragma once

#include "RenderPass.h"
#include "Graphics/Buffers/ShadowMap.h"
#include "Graphics/Shaders/Shader.h"
#include <memory>
#include <glm/glm.hpp>

class ShadowPass : public RenderPass {
public:
    // shadowResolution is the width/height of the shadow map.
    ShadowPass(const std::shared_ptr<Scene::Scene>& scene, GLsizei shadowResolution);
    ~ShadowPass();

    // Render the scene depth from the light's point of view.
    void Execute(const std::shared_ptr<Scene::Scene>& scene) override;

    // Not used for this pass.
    void UpdateFramebuffer(std::shared_ptr<graphics::FrameBuffer> framebuffer) override {}

    // Retrieve the shadow map.
    std::shared_ptr<graphics::ShadowMap> GetShadowMap() const { return shadowMap_; }

private:
    std::shared_ptr<graphics::ShadowMap> shadowMap_;
    std::shared_ptr<graphics::Shader> shadowShader_;
    glm::mat4 shadowMatrix_;
    bool calculated_ = false;
};