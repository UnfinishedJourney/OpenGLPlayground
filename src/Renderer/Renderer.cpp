#include "Renderer.h"
#include "Renderer/Passes/GeometryPass.h"
#include "Renderer/Passes/GridPass.h"
#include "Renderer/Passes/TerrainPass.h"
#include "Renderer/Passes/SkyBoxPass.h"
#include "Renderer/Passes/PostProcessingPass.h"
#include "Renderer/Passes/DebugLightsPass.h"
#include "Graphics/Effects/PostProcessingEffects/EdgeDetectionEffect.h"
#include "Graphics/Effects/EffectsManager.h"
#include "Utilities/Logger.h"
#include <glad/glad.h>
#include "Utilities/ProfilerMacros.h"

Renderer::Renderer()
    : m_Width(800), m_Height(600), m_CurrentScene(nullptr)
{
}

Renderer::~Renderer()
{
}

void Renderer::Initialize(int width, int height)
{
    // Potentially set up your GL states, etc.
    m_Width = width;
    m_Height = height;
}

void Renderer::RenderScene(const std::shared_ptr<Scene>& scene)
{
    PROFILE_FUNCTION(Blue);

    {
        PROFILE_BLOCK("Clear Framebuffer", Yellow);
        Clear();
    }

    // If scene changed, re-init passes
    if (scene != m_CurrentScene) {
        PROFILE_BLOCK("Initialize New Scene Passes", Yellow);
        m_CurrentScene = scene;
        InitializePassesForScene(scene);
    }

    {
        PROFILE_BLOCK("Execute Render Passes", Yellow);
        for (auto& pass : m_RenderPasses) {
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

void Renderer::InitializePassesForScene(const std::shared_ptr<Scene>& scene)
{
    PROFILE_FUNCTION(Cyan);

    m_RenderPasses.clear();

    auto framebuffer = CreateFramebufferForScene(scene, m_Width, m_Height);

    // 1) Geometry Pass
    {
        m_RenderPasses.push_back(std::make_unique<GeometryPass>(framebuffer, scene));
    }
    if (scene->GetBSkybox()) {
        m_RenderPasses.push_back(std::make_unique<SkyBoxPass>(framebuffer, scene));
    }

    // 2) Optional Grid
    if (scene->GetBGrid()) {
        m_RenderPasses.push_back(std::make_unique<GridPass>(framebuffer, scene));
    }

    // 3) Optional debug lights
    if (scene->GetBDebugLights()) {
        m_RenderPasses.push_back(std::make_unique<DebugLightsPass>(framebuffer, scene));
    }

    // 4) Post-processing
    {
        auto ppPass = std::make_unique<PostProcessingPass>(framebuffer, scene);
        auto& effectManager = EffectsManager::GetInstance();
        auto effect = effectManager.GetEffect(scene->GetPostProcessingEffect());
        ppPass->SetPostProcessingEffect(effect);
        m_RenderPasses.push_back(std::move(ppPass));
    }
}

std::shared_ptr<FrameBuffer> Renderer::CreateFramebufferForScene(const std::shared_ptr<Scene>& scene,
    int width, int height)
{
    PROFILE_FUNCTION(Yellow);

    std::vector<FrameBufferTextureAttachment> colorAttachments = {
        { GL_COLOR_ATTACHMENT0, GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE }  //for gamma-correction
    };

    auto framebuffer = std::make_shared<FrameBuffer>(width, height, colorAttachments, true);
    glEnable(GL_FRAMEBUFFER_SRGB);
    return framebuffer;
}

void Renderer::OnWindowResize(int width, int height)
{
    PROFILE_FUNCTION(Green);
    m_Width = width;
    m_Height = height;

    if (m_CurrentScene) {
        PROFILE_BLOCK("Recreate Framebuffer on Resize", Yellow);
        auto framebuffer = CreateFramebufferForScene(m_CurrentScene, m_Width, m_Height);
        for (auto& pass : m_RenderPasses) {
            pass->UpdateFramebuffer(framebuffer);
        }
    }

    {
        PROFILE_BLOCK("Handle Effects Manager Resize", Yellow);
        auto& effectManager = EffectsManager::GetInstance();
        effectManager.OnWindowResize(m_Width, m_Height);
    }
}