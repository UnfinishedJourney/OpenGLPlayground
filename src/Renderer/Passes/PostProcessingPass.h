#pragma once

#include "RenderPass.h"
#include "Graphics/Buffers/FrameBuffer.h"
#include "Graphics/Effects/PostProcessingEffects/PostProcessingEffect.h"
#include <memory>

class PostProcessingPass : public RenderPass {
public:
    PostProcessingPass(std::shared_ptr<graphics::FrameBuffer> /*ignored*/,
        const std::shared_ptr<Scene::Scene>& scene);
    ~PostProcessingPass();

    void Execute(const std::shared_ptr<Scene::Scene>& scene) override;
    void UpdateFramebuffer(std::shared_ptr<graphics::FrameBuffer> framebuffer) override;

    void SetPostProcessingEffect(const std::shared_ptr<PostProcessingEffect>& effect);
    void SetSourceFramebuffer(std::shared_ptr<graphics::FrameBuffer> sourceFBO);

private:
    std::shared_ptr<graphics::FrameBuffer> sourceFBO_; // Resolved FBO
    std::shared_ptr<PostProcessingEffect> effect_;
};