#pragma once

#include "RenderPass.h"
#include "Graphics/Buffers/FrameBuffer.h"

class PostProcessingPass : public RenderPass
{
public:
    PostProcessingPass(std::shared_ptr<FrameBuffer> framebuffer, const std::shared_ptr<Scene>& scene);
    ~PostProcessingPass();
    void Execute(const std::shared_ptr<Scene>& scene) override;

    void UpdateFramebuffer(std::shared_ptr<FrameBuffer> framebuffer) override;

private:
    void SetupFullscreenQuad();
    std::shared_ptr<FrameBuffer> m_Framebuffer;
    GLuint m_FullscreenQuadVAO = 0;
    GLuint m_FullscreenQuadVBO = 0;
};