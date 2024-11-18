#pragma once

#include <memory>
#include <vector>
#include "Scene/Scene.h"
#include "Renderer/Passes/RenderPass.h"
#include "Graphics/Buffers/FrameBuffer.h"

class Renderer
{
public:
    Renderer();
    ~Renderer();

    void Initialize(int width, int height);
    void RenderScene(const std::shared_ptr<Scene>& scene);
    void OnWindowResize(int width, int height);
    void Clear(float r = 0.3f, float g = 0.2f, float b = 0.8f, float a = 1.0f) const;

private:
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void InitializePassesForScene(const std::shared_ptr<Scene>& scene);
    std::shared_ptr<FrameBuffer> CreateFramebufferForScene(const std::shared_ptr<Scene>& scene, int width, int height);

    int m_Width;
    int m_Height;
    std::vector<std::unique_ptr<RenderPass>> m_RenderPasses;
    std::shared_ptr<Scene> m_CurrentScene;
};