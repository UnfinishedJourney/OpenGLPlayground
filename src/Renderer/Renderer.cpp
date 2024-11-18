﻿#include "Renderer.h"
#include "Renderer/Passes/GeometryPass.h"
#include "Renderer/Passes/PostProcessingPass.h"
#include "Renderer/Passes/DebugLightsPass.h"
#include "Graphics/Effects/PostProcessingEffects/EdgeDetectionEffect.h"
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
    auto logger = Logger::GetLogger();
    logger->info("Initializing Renderer.");

    m_Width = width;
    m_Height = height;
}

void Renderer::RenderScene(const std::shared_ptr<Scene>& scene)
{
    Clear();
    // If the scene has changed, reinitialize passes
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
    // Assuming m_Renderer has a method to get the PostProcessingPass

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