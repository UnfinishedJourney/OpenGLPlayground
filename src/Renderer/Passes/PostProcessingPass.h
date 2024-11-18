#pragma once

#include "RenderPass.h"
#include "Graphics/Buffers/FrameBuffer.h"
#include "Graphics/Effects/PostProcessingEffects/PostProcessingEffect.h"
#include <memory>

class PostProcessingPass : public RenderPass
{
public:
    PostProcessingPass(std::shared_ptr<FrameBuffer> framebuffer, const std::shared_ptr<Scene>& scene);
    ~PostProcessingPass();

    void Execute(const std::shared_ptr<Scene>& scene) override;
    void UpdateFramebuffer(std::shared_ptr<FrameBuffer> framebuffer) override;

    void SetPostProcessingEffect(const std::shared_ptr<PostProcessingEffect>& effect);

private:
    std::shared_ptr<FrameBuffer> m_Framebuffer;
    std::shared_ptr<PostProcessingEffect> m_Effect;
};