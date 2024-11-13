#pragma once

#include "Renderer/RenderState.h"
#include "Scene/Scene.h"

class RenderPass {
public:
    virtual void Setup() = 0;
    virtual void Render(const Scene& scene) = 0;
    virtual void Teardown() = 0;
    virtual ~RenderPass() = default;

protected:
    RenderState m_RenderState;
};