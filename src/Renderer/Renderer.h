#pragma once

#include <memory>
#include <vector>
#include "Scene/Scene.h"
#include "Renderer/Passes/RenderPass.h"
#include "Graphics/Buffers/FrameBuffer.h"

class Renderer
{
public:
    static Renderer& GetInstance()
    {
        static Renderer instance;
        return instance;
    }

    // Initialize the renderer with the window dimensions
    void Initialize(int width, int height);

    // Render the given scene
    void RenderScene(const std::shared_ptr<Scene>& scene);

    // Handle window resizing
    void OnWindowResize(int width, int height);

    void Clear(float r = 0.3f, float g = 0.2f, float b = 0.8f, float a = 1.0f) const;

private:
    Renderer();
    ~Renderer();

    // Delete copy constructor and assignment operator
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    // Setup the fullscreen quad for post-processing
    void SetupFullscreenQuad();

    // Initialize passes for the scene
    void InitializePassesForScene(const std::shared_ptr<Scene>& scene);

    // Create framebuffer for the scene
    std::shared_ptr<FrameBuffer> CreateFramebufferForScene(const std::shared_ptr<Scene>& scene, int width, int height);

    // Window dimensions
    int m_Width;
    int m_Height;

    // Fullscreen quad VAO and VBO
    GLuint m_FullscreenQuadVAO = 0;
    GLuint m_FullscreenQuadVBO = 0;

    // Render passes
    std::vector<std::unique_ptr<RenderPass>> m_RenderPasses;

    // Current scene
    std::shared_ptr<Scene> m_CurrentScene;
};