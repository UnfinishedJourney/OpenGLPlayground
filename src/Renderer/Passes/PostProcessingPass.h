#pragma once
#include "Renderer/Passes/RenderPass.h"
#include "Graphics/Effects/PostProcessingEffects/PostProcessingEffect.h"

class PostProcessingPass : public RenderPass
{
public:
    PostProcessingPass(std::shared_ptr<FrameBuffer> /*ignored*/,
        const std::shared_ptr<Scene>& scene);
    ~PostProcessingPass();

    void Execute(const std::shared_ptr<Scene>& scene) override;
    void UpdateFramebuffer(std::shared_ptr<FrameBuffer> framebuffer) override;

    void SetPostProcessingEffect(const std::shared_ptr<PostProcessingEffect>& effect);

    /**
     * @brief Sets the single-sample FBO whose texture we'll sample from.
     */
    void SetSourceFramebuffer(std::shared_ptr<FrameBuffer> sourceFBO);

private:
    std::shared_ptr<FrameBuffer> m_SourceFBO;  // The resolved FBO
    std::shared_ptr<PostProcessingEffect> m_Effect;
};
