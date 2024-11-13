#pragma once

#include "RenderPass.h"
#include "Graphics/Buffers/FrameBuffer.h"

// GeometryPass renders the scene's geometry into the framebuffer
class GeometryPass : public RenderPass
{
public:
    GeometryPass(std::shared_ptr<FrameBuffer> framebuffer, const std::shared_ptr<Scene>& scene);

    void Execute(const std::shared_ptr<Scene>& scene) override;

    void UpdateFramebuffer(std::shared_ptr<FrameBuffer> framebuffer) override;

    ~GeometryPass();

private:
    std::shared_ptr<FrameBuffer> m_Framebuffer;

    // Scene-specific resources
    GLuint m_LightSSBO;

    void InitializeSceneResources(const std::shared_ptr<Scene>& scene);
};