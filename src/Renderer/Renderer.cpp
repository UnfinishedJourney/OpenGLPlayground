#include "Renderer.h"
#include "Renderer/Passes/GeometryPass.h"
#include "Renderer/Passes/GridPass.h"
#include "Renderer/Passes/TerrainPass.h"
#include "Renderer/Passes/PostProcessingPass.h"
#include "Renderer/Passes/DebugLightsPass.h"
#include "Graphics/Effects/PostProcessingEffects/EdgeDetectionEffect.h"
#include "Graphics/Effects/EffectsManager.h"
#include "Utilities/Logger.h"
#include <glad/glad.h>

Renderer::Renderer()
    : m_Width(800), m_Height(600), m_CurrentScene(nullptr)
{
}

Renderer::~Renderer()
{
}

void Renderer::Initialize(int width, int height)
{
    m_Width = width;
    m_Height = height;
}

void Renderer::RenderScene(const std::shared_ptr<Scene>& scene)
{
    Clear();

    if (scene != m_CurrentScene)
    {
        m_CurrentScene = scene;
        InitializePassesForScene(scene);
    }

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
    m_RenderPasses.clear();

    auto framebuffer = CreateFramebufferForScene(scene, m_Width, m_Height);

    m_RenderPasses.push_back(std::make_unique<GeometryPass>(framebuffer, scene));

    if (scene->GetBGrid())
    {
        m_RenderPasses.push_back(std::make_unique<GridPass>(framebuffer, scene));
    }

    if (scene->GetTerrainHeightMap())
    {
        m_RenderPasses.push_back(std::make_unique<TerrainPass>(framebuffer, scene));
    }

    if (scene->GetBDebugLights())
    {
        m_RenderPasses.push_back(std::make_unique<DebugLightsPass>(framebuffer, scene));
    }

    auto ppPass = std::make_unique<PostProcessingPass>(framebuffer, scene);
    auto& effectManager = EffectsManager::GetInstance();
    auto edgeEffect = effectManager.GetEffect(scene->GetPostProcessingEffect());
    ppPass->SetPostProcessingEffect(edgeEffect);
    m_RenderPasses.push_back(std::move(ppPass));
}

std::shared_ptr<FrameBuffer> Renderer::CreateFramebufferForScene(const std::shared_ptr<Scene>& scene, int width, int height)
{
    std::vector<FrameBufferTextureAttachment> colorAttachments = {
        { GL_COLOR_ATTACHMENT0, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE }       //will need more attachments later
    };

    auto framebuffer = std::make_shared<FrameBuffer>(width, height, colorAttachments, true);

    return framebuffer;
}

void Renderer::OnWindowResize(int width, int height)
{
    m_Width = width;
    m_Height = height;

    if (m_CurrentScene)
    {
        auto framebuffer = CreateFramebufferForScene(m_CurrentScene, m_Width, m_Height);

        for (auto& pass : m_RenderPasses)
        {
            pass->UpdateFramebuffer(framebuffer);
        }
    }

    auto& effectManager = EffectsManager::GetInstance();
    effectManager.OnWindowResize(m_Width, m_Height);
}