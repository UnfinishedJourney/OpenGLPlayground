#pragma once

#include "RenderPass.h"
#include "Graphics/Buffers/FrameBuffer.h"
#include "Graphics/Buffers/MeshBuffer.h"
#include <memory>

class SkyBoxPass : public RenderPass {
public:
    SkyBoxPass(std::shared_ptr<graphics::FrameBuffer> framebuffer,
        const std::shared_ptr<Scene::Scene>& scene);
    ~SkyBoxPass();

    void Execute(const std::shared_ptr<Scene::Scene>& scene) override;
    void UpdateFramebuffer(std::shared_ptr<graphics::FrameBuffer> framebuffer) override;

private:
    void InitializeSceneResources(const std::shared_ptr<Scene::Scene>& scene);

    std::shared_ptr<graphics::FrameBuffer> framebuffer_;
    std::shared_ptr<graphics::MeshBuffer> skyboxMeshBuffer_;
};
