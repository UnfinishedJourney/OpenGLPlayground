#pragma once

#include "RenderPass.h"
#include "Graphics/Buffers/FrameBuffer.h"
#include <memory>
#include <glm/glm.hpp>

class GeometryPass : public RenderPass {
public:
    GeometryPass(std::shared_ptr<graphics::FrameBuffer> framebuffer,
        const std::shared_ptr<Scene::Scene>& scene);
    ~GeometryPass() = default;

    void Execute(const std::shared_ptr<Scene::Scene>& scene) override;
    void UpdateFramebuffer(std::shared_ptr<graphics::FrameBuffer> framebuffer) override;

private:
    std::shared_ptr<graphics::FrameBuffer> framebuffer_;
    bool shadowed_ = false;
    glm::mat4 shadowMatrix_ = glm::mat4(1.0f);
};