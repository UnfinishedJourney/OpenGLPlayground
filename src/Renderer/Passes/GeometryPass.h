#pragma once

#include "RenderPass.h"
#include "Graphics/Buffers/FrameBuffer.h"
#include "Scene/Scene.h"

class GeometryPass : public RenderPass
{
public:
    GeometryPass(std::shared_ptr<FrameBuffer> framebuffer, const std::shared_ptr<Scene>& scene);
    ~GeometryPass() = default;

    void Execute(const std::shared_ptr<Scene>& scene) override;
    void UpdateFramebuffer(std::shared_ptr<FrameBuffer> framebuffer) override;

private:
    std::shared_ptr<FrameBuffer> m_Framebuffer;
};