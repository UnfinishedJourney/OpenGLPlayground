#pragma once

#include <memory>
#include "Scene/Scene.h"
#include "Graphics/Buffers/FrameBuffer.h"

// Base class for render passes
class RenderPass
{
public:
    virtual ~RenderPass() = default;

    // Execute the render pass with the given scene
    virtual void Execute(const std::shared_ptr<Scene::Scene>& scene) = 0;

    // Update the framebuffer (e.g., when window size changes)
    virtual void UpdateFramebuffer(std::shared_ptr<FrameBuffer> framebuffer) = 0;
};