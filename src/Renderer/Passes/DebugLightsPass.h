#pragma once

#include "RenderPass.h"
#include "Graphics/Buffers/FrameBuffer.h"
#include "Graphics/Buffers/MeshBuffer.h"
#include <memory>

class DebugLightsPass : public RenderPass {
public:
    DebugLightsPass(std::shared_ptr<graphics::FrameBuffer> framebuffer,
        const std::shared_ptr<Scene::Scene>& scene);
    ~DebugLightsPass();

    void Execute(const std::shared_ptr<Scene::Scene>& scene) override;
    void UpdateFramebuffer(std::shared_ptr<graphics::FrameBuffer> framebuffer) override;

private:
    std::shared_ptr<graphics::FrameBuffer> framebuffer_;
    std::shared_ptr<graphics::MeshBuffer> lightSphereMeshBuffer_;

    void InitializeSceneResources(const std::shared_ptr<Scene::Scene>& scene);
};