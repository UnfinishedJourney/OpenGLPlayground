#pragma once

#include "RenderPass.h"
#include "Graphics/Buffers/FrameBuffer.h"
#include "Graphics/Buffers/MeshBuffer.h"
#include <memory>

class GridPass : public RenderPass {
public:
    GridPass(std::shared_ptr<graphics::FrameBuffer> framebuffer, const std::shared_ptr<Scene::Scene>& scene);
    ~GridPass();

    void Execute(const std::shared_ptr<Scene::Scene>& scene) override;
    void UpdateFramebuffer(std::shared_ptr<graphics::FrameBuffer> framebuffer) override;

private:
    std::shared_ptr<graphics::FrameBuffer> m_Framebuffer;
    std::shared_ptr<graphics::MeshBuffer> m_GridMeshBuffer;
};