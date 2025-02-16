#pragma once

#include <memory>
#include <vector>

#include "Scene/Scene.h"
#include "Renderer/Passes/RenderPass.h"
#include "Graphics/Buffers/FrameBuffer.h"
#include "Renderer/Passes/PostProcessingPass.h"
#include "Renderer/Passes/ShadowPass.h"

/**
 * @brief Main renderer orchestrating:
 *  - MSAA rendering to an off-screen FBO
 *  - Resolving to a single-sample FBO
 *  - Optional post-processing steps (e.g. edge-detection)
 *  - Final output to the default framebuffer (screen).
 */
class Renderer {
public:
    Renderer();
    ~Renderer();

    void Initialize(int width, int height);
    void RenderScene(const std::shared_ptr<Scene::Scene>& scene);
    void OnWindowResize(int width, int height);
    void Clear(float r = 0.3f, float g = 0.2f, float b = 0.8f, float a = 1.0f) const;

private:
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void CreateFramebuffersForScene(const std::shared_ptr<Scene::Scene>& scene,
        int width, int height);
    void InitializePassesForScene(const std::shared_ptr<Scene::Scene>& scene);

private:
    int width_ = 800;
    int height_ = 600;

    std::vector<std::unique_ptr<RenderPass>> geometryPasses_;
    std::vector<std::unique_ptr<PostProcessingPass>> postProcessingPasses_;
    std::vector<std::unique_ptr<ShadowPass>> shadowPasses_;

    std::shared_ptr<Scene::Scene> currentScene_;

    // Off-screen framebuffers.
    std::shared_ptr<graphics::FrameBuffer> msaaFBO_;
    std::shared_ptr<graphics::FrameBuffer> resolvedFBO_;
};
