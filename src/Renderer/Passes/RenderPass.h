#pragma once

#include <memory>
#include "Scene/Scene.h"
#include "Graphics/Buffers/FrameBuffer.h"

class RenderPass {
public:
    virtual ~RenderPass() = default;

    // Execute the render pass using the given scene.
    virtual void Execute(const std::shared_ptr<Scene::Scene>& scene) = 0;

    // Update the framebuffer (e.g. on window resize).
    virtual void UpdateFramebuffer(std::shared_ptr<graphics::FrameBuffer> framebuffer) = 0;
};