#pragma once

#include "RenderPass.h"
#include "Graphics/Buffers/FrameBuffer.h"
#include "Graphics/Buffers/MeshBuffer.h"
#include "Scene/Scene.h"  // so we can get scene->BindFrameDataUBO(), etc.

#include <memory>

class SkyBoxPass : public RenderPass
{
public:
    SkyBoxPass(std::shared_ptr<FrameBuffer> framebuffer,
        const std::shared_ptr<Scene>& scene);

    void Execute(const std::shared_ptr<Scene>& scene) override;
    void UpdateFramebuffer(std::shared_ptr<FrameBuffer> framebuffer) override;

    ~SkyBoxPass();

private:
    void InitializeSceneResources(const std::shared_ptr<Scene>& scene);

    std::shared_ptr<FrameBuffer> m_Framebuffer;
    std::shared_ptr<Graphics::MeshBuffer>  m_SkyboxMeshBuffer;
};