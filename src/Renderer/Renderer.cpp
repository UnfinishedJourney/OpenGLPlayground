#include "Renderer.h"

// Example passes (SkyBoxPass, GeometryPass, etc.)
#include "Renderer/Passes/GeometryPass.h"
#include "Renderer/Passes/GridPass.h"
#include "Renderer/Passes/SkyBoxPass.h"
#include "Renderer/Passes/DebugLightsPass.h"

#include "Graphics/Effects/PostProcessingEffects/EdgeDetectionEffect.h"
#include "Graphics/Effects/EffectsManager.h"
#include "Utilities/Logger.h"
#include "Utilities/ProfilerMacros.h"

#include <glad/glad.h>

Renderer::Renderer()
    : m_Width(800)
    , m_Height(600)
    , m_CurrentScene(nullptr)
{
}

Renderer::~Renderer()
{
}

void Renderer::Initialize(int width, int height)
{
    m_Width = width;
    m_Height = height;
    // Optionally set GL states here, e.g.:
    // glEnable(GL_DEPTH_TEST);
    // glEnable(GL_CULL_FACE);
    // etc.
}

void Renderer::RenderScene(const std::shared_ptr<Scene>& scene)
{
    PROFILE_FUNCTION(Blue);

    // 1) If the scene changed, re-init passes (which also re-creates FBOs)
    if (scene != m_CurrentScene) {
        PROFILE_BLOCK("Initialize New Scene Passes", Yellow);
        m_CurrentScene = scene;
        InitializePassesForScene(scene);
    }

    // 2) Clear the screen (the default/system framebuffer)
    {
        PROFILE_BLOCK("Clear Screen", Yellow);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        Clear();
    }

    // 3) Render into MSAA FBO: geometry, skybox, debug-lights, etc.
    //    We'll group them as "pre-postprocessing" passes.
    {
        PROFILE_BLOCK("Render into MSAA FBO", Cyan);

        if (m_MsaaFBO) {
            m_MsaaFBO->Bind();
            Clear(); // Очистка цвета и глубины в MSAA FBO
        }


        for (auto& pass : m_GeometryPasses) {
            pass->Execute(scene);
        }


        if (m_MsaaFBO) {
            m_MsaaFBO->Unbind();
        }
    }


    {
        PROFILE_BLOCK("Resolve MSAA FBO", Green);

        if (m_MsaaFBO && m_ResolvedFBO) {

            m_MsaaFBO->BlitTo(*m_ResolvedFBO, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        }
    }


    {
        PROFILE_BLOCK("Post-Processing", Magenta);

        for (auto& pass : m_PostProcessingPasses) {
            pass->Execute(scene);
        }
    }

    // 6) At this point, you have presumably rendered the final image to the default framebuffer,
    //    so you can present on screen. (If your PostProcessingPass is drawing to 0, that’s it.)
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

    if (m_CurrentScene) {
        // Re-create FBOs for the current scene
        PROFILE_BLOCK("Recreate Framebuffers on Resize", Yellow);
        CreateFramebuffersForScene(m_CurrentScene, m_Width, m_Height);

        // Update pass framebuffers
        for (auto& pass : m_GeometryPasses) {
            pass->UpdateFramebuffer(m_MsaaFBO);
        }

        for (auto& pass : m_PostProcessingPasses) {
            pass->SetSourceFramebuffer(m_ResolvedFBO);
        }
    }

    // Also tell any effect managers or other managers about the new size
    auto& effectManager = EffectsManager::GetInstance();
    effectManager.OnWindowResize(m_Width, m_Height);
}

void Renderer::InitializePassesForScene(const std::shared_ptr<Scene>& scene)
{
    PROFILE_FUNCTION(Cyan);

    // Clear old passes
    m_GeometryPasses.clear();
    m_PostProcessingPasses.clear();

    // (Re-)Create MSAA FBO + resolved FBO
    CreateFramebuffersForScene(scene, m_Width, m_Height);

    // For each pass that draws geometry, we pass m_MsaaFBO
    // For post-processing pass, we'll reference the resolved FBO
    // in the pass's `SetSourceFramebuffer(...)` method or constructor.

    // 1) Skybox pass (if needed)
    if (scene->GetBSkybox()) {
        m_GeometryPasses.push_back(std::make_unique<SkyBoxPass>(m_MsaaFBO, scene));
    }

    // 2) Geometry pass
    m_GeometryPasses.push_back(std::make_unique<GeometryPass>(m_MsaaFBO, scene));

    // 3) Grid pass
    if (scene->GetBGrid()) {
        m_GeometryPasses.push_back(std::make_unique<GridPass>(m_MsaaFBO, scene));
    }

    // 4) Debug lights
    if (scene->GetBDebugLights()) {
        m_GeometryPasses.push_back(std::make_unique<DebugLightsPass>(m_MsaaFBO, scene));
    }

    // 5) Post-processing pass
    {
        auto ppPass = std::make_unique<PostProcessingPass>(m_ResolvedFBO, scene);
        auto& effectManager = EffectsManager::GetInstance();
        auto effect = effectManager.GetEffect(scene->GetPostProcessingEffect());
        ppPass->SetPostProcessingEffect(effect);

        // We'll set the actual source FBO later (in the render loop or OnWindowResize).
        // Or we can call ppPass->SetSourceFramebuffer(m_ResolvedFBO) here.
        ppPass->SetSourceFramebuffer(m_ResolvedFBO);

        m_PostProcessingPasses.push_back(std::move(ppPass));
    }
}

void Renderer::CreateFramebuffersForScene(const std::shared_ptr<Scene>& scene,
    int width, int height)
{
    PROFILE_FUNCTION(Yellow);

    // 1) CREATE the MSAA FBO
    //   - e.g. 4 samples
    int samples = 4;
    std::vector<FrameBufferTextureAttachment> msaaColorAttachments = {
        { GL_COLOR_ATTACHMENT0, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE }
    };

    m_MsaaFBO = std::make_shared<FrameBuffer>(width,
        height,
        msaaColorAttachments,
        /*hasDepth=*/true,
        /*samples=*/samples);

    // 2) CREATE the single-sample FBO for the resolved color
    std::vector<FrameBufferTextureAttachment> resolvedColorAttachments = {
        { GL_COLOR_ATTACHMENT0, GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE }
    };

    m_ResolvedFBO = std::make_shared<FrameBuffer>(width,
        height,
        resolvedColorAttachments,
        /*hasDepth=*/false,
        /*samples=*/1);

    glEnable(GL_FRAMEBUFFER_SRGB);

    Logger::GetLogger()->info("Created MSAA FBO ({} samples) and resolved FBO (1 sample).",
        samples);
}