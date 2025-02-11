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
class Renderer
{
public:
    Renderer();
    ~Renderer();

    /**
     * @brief Initialize GL states or other things needed once at startup.
     * @param width  Initial viewport width
     * @param height Initial viewport height
     */
    void Initialize(int width, int height);

    /**
     * @brief Renders the given scene using multiple passes:
     *        1) geometry/skybox/etc. into MSAA FBO
     *        2) blit from MSAA FBO -> single-sample FBO
     *        3) post-processing pass using single-sample texture
     */
    void RenderScene(const std::shared_ptr<Scene::Scene>& scene);

    /**
     * @brief Called when the window is resized. Recreates FBOs and passes if needed.
     */
    void OnWindowResize(int width, int height);

    /**
     * @brief Clears the *currently bound* framebuffer.
     */
    void Clear(float r = 0.3f, float g = 0.2f, float b = 0.8f, float a = 1.0f) const;

private:
    // Non-copyable
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    /**
     * @brief Creates or re-creates the MSAA and single-sample framebuffers
     *        for rendering the given scene at the given width/height.
     */
    void CreateFramebuffersForScene(const std::shared_ptr<Scene::Scene>& scene,
        int width,
        int height);

    /**
     * @brief (Re)initializes all render passes for the given scene,
     *        clearing old passes and building new ones.
     */
    void InitializePassesForScene(const std::shared_ptr<Scene::Scene>& scene);

private:
    int m_Width = 800;
    int m_Height = 600;

    // We store geometry passes (i.e. skybox, debug-lights, main geometry)
    std::vector<std::unique_ptr<RenderPass>>       m_GeometryPasses;

    // We store post-processing passes (e.g. tone-map, edge-detection, etc.)
    std::vector<std::unique_ptr<PostProcessingPass>> m_PostProcessingPasses;

    std::vector<std::unique_ptr<ShadowPass>>       m_ShadowPasses;

    // The current scene this renderer is rendering
    std::shared_ptr<Scene::Scene> m_CurrentScene;

    // MSAA (multisample) FBO for rendering geometry
    std::shared_ptr<FrameBuffer> m_MsaaFBO;

    // Single-sample FBO for resolved color
    std::shared_ptr<FrameBuffer> m_ResolvedFBO;
};