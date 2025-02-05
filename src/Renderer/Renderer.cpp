#include "Renderer.h"

#include <glad/glad.h>
#include <stdexcept>
#include <algorithm>

// Example passes
#include "Renderer/Passes/GeometryPass.h"
#include "Renderer/Passes/GridPass.h"
#include "Renderer/Passes/SkyBoxPass.h"
#include "Renderer/Passes/DebugLightsPass.h"
#include "Renderer/Passes/ShadowPass.h"

#include "Graphics/Effects/PostProcessingEffects/EdgeDetectionEffect.h"
#include "Graphics/Effects/EffectsManager.h"
#include "Utilities/Logger.h"
#include "Utilities/ProfilerMacros.h"
#include "Graphics/Buffers/FrameBuffer.h"
#include "Renderer/Passes/PostProcessingPass.h"
#include "Scene/Scene.h"

#include <glm/gtc/matrix_transform.hpp>

Renderer::Renderer()
    : m_Width(800)
    , m_Height(600)
{
    // Possibly set some defaults, e.g. glEnable(GL_DEPTH_TEST) in Initialize()
}

Renderer::~Renderer() = default;

void Renderer::Initialize(int width, int height)
{
    m_Width = width;
    m_Height = height;

    // You can also do global GL states:
    // glEnable(GL_DEPTH_TEST);
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_BACK);
}

void Renderer::RenderScene(const std::shared_ptr<Scene>& scene)
{
    PROFILE_FUNCTION(Blue);

    // If the scene changed, we (re)initialize passes and FBOs
    if (scene != m_CurrentScene) {
        PROFILE_BLOCK("Initialize New Scene Passes", Yellow);
        m_CurrentScene = scene;
        InitializePassesForScene(scene);
    }

    // 1) Clear the system/default framebuffer (not strictly needed, but helpful).
    {
        PROFILE_BLOCK("Clear Screen", Yellow);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        Clear(0.3f, 0.2f, 0.8f, 1.0f);
    }

    for (auto& pass : m_ShadowPasses) {
        pass->Execute(scene);
    }

    // 2) Render geometry passes into the MSAA FBO
    {
        PROFILE_BLOCK("Render into MSAA FBO", Cyan);

        if (m_MsaaFBO) {
            m_MsaaFBO->Bind();
            Clear(); // Clears color/depth in MSAA FBO
        }
        for (auto& pass : m_GeometryPasses) {
            pass->Execute(scene);
        }
        if (m_MsaaFBO) {
            m_MsaaFBO->Unbind();
        }
    }

    // 3) Resolve MSAA -> single-sample FBO
    {
        PROFILE_BLOCK("Resolve MSAA FBO", Green);

        if (m_MsaaFBO && m_ResolvedFBO) {
            // Typically we only need GL_COLOR_BUFFER_BIT
            m_MsaaFBO->BlitTo(*m_ResolvedFBO, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        }
    }

    // 4) Post-processing passes using the resolved FBO texture
    {
        PROFILE_BLOCK("Post-Processing", Magenta);

        for (auto& pass : m_PostProcessingPasses) {
            pass->Execute(scene);
        }
    }
}

void Renderer::Clear(float r, float g, float b, float a) const
{
    PROFILE_FUNCTION(Yellow);

    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::OnWindowResize(int width, int height)
{
    PROFILE_FUNCTION(Green);

    m_Width = width;
    m_Height = height;

    // Recreate the FBOs if we already have a scene loaded
    if (m_CurrentScene) {
        PROFILE_BLOCK("Recreate FBO on Resize", Yellow);
        CreateFramebuffersForScene(m_CurrentScene, width, height);

        // Update pass framebuffers if needed
        for (auto& pass : m_GeometryPasses) {
            pass->UpdateFramebuffer(m_MsaaFBO);
        }
        for (auto& pass : m_PostProcessingPasses) {
            pass->SetSourceFramebuffer(m_ResolvedFBO);
        }
    }

    // Also inform effect manager
    EffectsManager::GetInstance().OnWindowResize(width, height);
}

void Renderer::InitializePassesForScene(const std::shared_ptr<Scene>& scene)
{
    PROFILE_FUNCTION(Cyan);

    // Clear old passes
    m_GeometryPasses.clear();
    m_PostProcessingPasses.clear();

    // Recreate FBOs
    CreateFramebuffersForScene(scene, m_Width, m_Height);

    if (scene->GetShowShadows()) {
        m_ShadowPasses.push_back(std::make_unique<ShadowPass>(2*2048));
    }

    // 1) Possibly add a skybox pass
    if (scene->GetSkyboxEnabled()) {
        m_GeometryPasses.push_back(std::make_unique<SkyBoxPass>(m_MsaaFBO, scene));
    }

    // 2) Main geometry pass
    m_GeometryPasses.push_back(std::make_unique<GeometryPass>(m_MsaaFBO, scene));

    // 3) Grid pass
    if (scene->GetShowGrid()) {
        m_GeometryPasses.push_back(std::make_unique<GridPass>(m_MsaaFBO, scene));
    }

    // 4) Debug lights pass
    if (scene->GetShowDebugLights()) {
        m_GeometryPasses.push_back(std::make_unique<DebugLightsPass>(m_MsaaFBO, scene));
    }

    // 5) Post-processing
    {
        auto ppPass = std::make_unique<PostProcessingPass>(m_ResolvedFBO, scene);
        auto effect = EffectsManager::GetInstance().GetEffect(scene->GetPostProcessingEffect());
        ppPass->SetPostProcessingEffect(effect);
        ppPass->SetSourceFramebuffer(m_ResolvedFBO);
        m_PostProcessingPasses.push_back(std::move(ppPass));
    }
}

void Renderer::CreateFramebuffersForScene(const std::shared_ptr<Scene>& scene,
    int width,
    int height)
{
    PROFILE_FUNCTION(Yellow);

    // MSAA FBO: choose a sample count, e.g., 4
    int samples = 4;

    // Typically one color attachment, RGBA8
    std::vector<FrameBufferTextureAttachment> msaaAttachments = {
        { GL_COLOR_ATTACHMENT0, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE }
    };

    m_MsaaFBO = std::make_shared<FrameBuffer>(
        width,
        height,
        msaaAttachments,
        /*hasDepth=*/true,
        /*samples=*/samples
    );

    // Single-sample FBO (resolved)
    std::vector<FrameBufferTextureAttachment> resolvedColorAttachment = {
        { GL_COLOR_ATTACHMENT0, GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE }
    };

    m_ResolvedFBO = std::make_shared<FrameBuffer>(
        width,
        height,
        resolvedColorAttachment,
        /*hasDepth=*/false,
        /*samples=*/1
    );

    // sRGB enabled
    //glEnable(GL_FRAMEBUFFER_SRGB);

    Logger::GetLogger()->info("Renderer: Created {}x{} MSAA FBO ({}) samples + resolved FBO(1 sample).",
        width, height, samples);
}