#pragma once

#include "RenderPass.h"
#include "Graphics/Buffers/FrameBuffer.h"

// PostProcessingPass applies post-processing effects and renders to the screen
class PostProcessingPass : public RenderPass
{
public:
    PostProcessingPass(GLuint fullscreenQuadVAO, std::shared_ptr<FrameBuffer> framebuffer, const std::shared_ptr<Scene>& scene);

    void Execute(const std::shared_ptr<Scene>& scene) override;

    void UpdateFramebuffer(std::shared_ptr<FrameBuffer> framebuffer) override;

private:
    GLuint m_FullscreenQuadVAO;
    std::shared_ptr<FrameBuffer> m_Framebuffer;
};