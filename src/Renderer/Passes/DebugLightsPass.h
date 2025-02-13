#pragma once

#include "RenderPass.h"
#include "Graphics/Buffers/FrameBuffer.h"
#include "Graphics/Buffers/MeshBuffer.h"
#include <memory>

// DebugLightsPass renders the scene's geometry into the framebuffer
class DebugLightsPass : public RenderPass
{
public:
    DebugLightsPass(std::shared_ptr<graphics::FrameBuffer> framebuffer, const std::shared_ptr<Scene::Scene>& scene);

    void Execute(const std::shared_ptr<Scene::Scene>& scene) override;

    void UpdateFramebuffer(std::shared_ptr<graphics::FrameBuffer> framebuffer) override;

    ~DebugLightsPass();

private:
    std::shared_ptr<graphics::FrameBuffer> m_Framebuffer;

    void InitializeSceneResources(const std::shared_ptr<Scene::Scene>& scene);
    std::shared_ptr<graphics::MeshBuffer> m_LightSphereMeshBuffer;
};