#include "Renderer.h"
#include "Renderer/Passes/GeometryPass.h"
#include "Renderer/Passes/PostProcessingPass.h"
#include "Renderer/Passes/DebugLightsPass.h"
#include "Utilities/Logger.h"
#include <glad/glad.h>

Renderer::Renderer()
    : m_Width(800), m_Height(600), m_CurrentScene(nullptr)
{
    // Constructor does nothing; initialization is in Initialize()
}

Renderer::~Renderer()
{
    if (m_FullscreenQuadVAO)
        glDeleteVertexArrays(1, &m_FullscreenQuadVAO);
    if (m_FullscreenQuadVBO)
        glDeleteBuffers(1, &m_FullscreenQuadVBO);
}

void Renderer::Initialize(int width, int height)
{
    auto logger = Logger::GetLogger();
    logger->info("Initializing Renderer.");

    m_Width = width;
    m_Height = height;

    // Setup the fullscreen quad for post-processing
    SetupFullscreenQuad();
}

void Renderer::SetupFullscreenQuad()
{
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,   0.0f, 1.0f,
        -1.0f, -1.0f,   0.0f, 0.0f,
         1.0f, -1.0f,   1.0f, 0.0f,

        -1.0f,  1.0f,   0.0f, 1.0f,
         1.0f, -1.0f,   1.0f, 0.0f,
         1.0f,  1.0f,   1.0f, 1.0f
    };

    glGenVertexArrays(1, &m_FullscreenQuadVAO);
    glGenBuffers(1, &m_FullscreenQuadVBO);
    glBindVertexArray(m_FullscreenQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_FullscreenQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    // TexCoord attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);

    Logger::GetLogger()->info("Fullscreen Quad VAO and VBO set up.");
}

void Renderer::RenderScene(const std::shared_ptr<Scene>& scene)
{
    // If the scene has changed, reinitialize passes
    if (scene != m_CurrentScene)
    {
        m_CurrentScene = scene;
        InitializePassesForScene(scene);
    }

    // Execute all render passes
    for (auto& pass : m_RenderPasses)
    {
        pass->Execute(scene);
    }
}

void Renderer::Clear(float r, float g, float b, float a) const
{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::InitializePassesForScene(const std::shared_ptr<Scene>& scene)
{
    // Clear existing passes
    m_RenderPasses.clear();

    // Create a framebuffer for the scene
    auto framebuffer = CreateFramebufferForScene(scene, m_Width, m_Height);

    // Initialize passes with the framebuffer and scene
    m_RenderPasses.push_back(std::make_unique<GeometryPass>(framebuffer, scene));
    m_RenderPasses.push_back(std::make_unique<DebugLightsPass>(framebuffer, scene));
    m_RenderPasses.push_back(std::make_unique<PostProcessingPass>(m_FullscreenQuadVAO, framebuffer, scene));

    // Add additional passes as needed
}

std::shared_ptr<FrameBuffer> Renderer::CreateFramebufferForScene(const std::shared_ptr<Scene>& scene, int width, int height)
{
    // Create framebuffer based on scene's requirements
    std::vector<FrameBufferTextureAttachment> colorAttachments = {
        { GL_COLOR_ATTACHMENT0, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE }
        // Add more attachments if needed
    };

    auto framebuffer = std::make_shared<FrameBuffer>(width, height, colorAttachments, true);
    //if (!framebuffer->IsComplete())
    //{
    //    Logger::GetLogger()->error("Framebuffer is not complete for the scene.");
    //}

    return framebuffer;
}

void Renderer::OnWindowResize(int width, int height)
{
    m_Width = width;
    m_Height = height;

    // Recreate framebuffer and update passes
    if (m_CurrentScene)
    {
        auto framebuffer = CreateFramebufferForScene(m_CurrentScene, m_Width, m_Height);

        // Update passes with the new framebuffer
        for (auto& pass : m_RenderPasses)
        {
            pass->UpdateFramebuffer(framebuffer);
        }
    }
}