#pragma once

#include <memory>
#include <vector>
#include "Scene/Scene.h"
#include "Renderer/Passes/RenderPass.h"
#include "Graphics/Buffers/FrameBuffer.h"
#include "Renderer/Passes/PostProcessingPass.h"

/**
 * @brief Main Renderer class that orchestrates all render passes,
 *        including MSAA FBO and a resolved (single-sample) FBO for post-processing.
 */
class Renderer
{
public:
    Renderer();
    ~Renderer();

    void Initialize(int width, int height);

    /**
     * @brief Renders the given scene using multiple passes:
     *        1) geometry/skybox/etc. into MSAA FBO
     *        2) blit from MSAA FBO -> single-sample FBO
     *        3) post-processing pass using single-sample texture
     */
    void RenderScene(const std::shared_ptr<Scene>& scene);

    /**
     * @brief Called when the window resizes; re-create FBOs and passes as needed.
     */
    void OnWindowResize(int width, int height);

    /**
     * @brief Clears the currently bound framebuffer.
     */
    void Clear(float r = 0.3f, float g = 0.2f, float b = 0.8f, float a = 1.0f) const;

private:
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    /**
     * @brief Initializes or re-initializes all passes for the given scene.
     */
    void InitializePassesForScene(const std::shared_ptr<Scene>& scene);

    /**
     * @brief (Re)creates the MSAA FBO and resolved FBO with the given dimensions
     *        and stores them in m_MsaaFBO and m_ResolvedFBO.
     */
    void CreateFramebuffersForScene(const std::shared_ptr<Scene>& scene,
        int width, int height);

private:
    int m_Width;
    int m_Height;

    // We keep track of all the passes we want to run:
    std::vector<std::unique_ptr<RenderPass>> m_GeometryPasses;
    std::vector<std::unique_ptr<PostProcessingPass>> m_PostProcessingPasses;

    // Current scene being rendered
    std::shared_ptr<Scene> m_CurrentScene;

    // The MULTISAMPLE FBO (4x MSAA for geometry)
    std::shared_ptr<FrameBuffer> m_MsaaFBO;

    // The RESOLVED single-sample FBO (to which we blit for post-processing)
    std::shared_ptr<FrameBuffer> m_ResolvedFBO;
};