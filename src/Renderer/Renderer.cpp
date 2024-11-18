#include "Renderer.h"
#include "Renderer/Passes/GeometryPass.h"
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

    // Initialize the EffectsManager
    EffectsManager::GetInstance().Initialize(width, height);
}

void Renderer::RenderScene(const std::shared_ptr<Scene>& scene)
{
    Clear();

    //// If the scene has changed, reinitialize passes
    //if (scene != m_CurrentScene)
    //{
    //    m_CurrentScene = scene;
    //    InitializePassesForScene(scene);
    //}

    //// Update the PostProcessingPass with the effect specified by the scene
    //auto postProcessingPass = std::find_if(m_RenderPasses.begin(), m_RenderPasses.end(),
    //    [](const std::unique_ptr<RenderPass>& pass) {
    //        return dynamic_cast<PostProcessingPass*>(pass.get()) != nullptr;
    //    });

    //if (postProcessingPass != m_RenderPasses.end())
    //{
    //    auto ppPass = dynamic_cast<PostProcessingPass*>(postProcessingPass->get());
    //    auto effectType = scene->GetPostProcessingEffect();
    //    auto effect = EffectsManager::GetInstance().GetEffect(effectType);
    //    ppPass->SetPostProcessingEffect(effect);
    //}

    //// Execute passes
    //for (auto& pass : m_RenderPasses)
    //{
    //    pass->Execute(scene);
    //}

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
    m_RenderPasses.push_back(std::make_unique<DebugLightsPass>(framebuffer, scene));
    auto PpPass = std::make_unique<PostProcessingPass>(framebuffer, scene);
    auto edgeEffect = std::make_shared<EdgeDetectionEffect>();
    edgeEffect->OnWindowResize(m_Width, m_Height);
    PpPass->SetPostProcessingEffect(edgeEffect);
    m_RenderPasses.push_back(std::move(PpPass));
}

std::shared_ptr<FrameBuffer> Renderer::CreateFramebufferForScene(const std::shared_ptr<Scene>& scene, int width, int height)
{
    std::vector<FrameBufferTextureAttachment> colorAttachments = {
        { GL_COLOR_ATTACHMENT0, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE }       //will need more attachments when needed
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
}