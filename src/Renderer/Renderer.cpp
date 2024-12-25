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
}

void Renderer::RenderScene(const std::shared_ptr<Scene>& scene)
{
    PROFILE_FUNCTION(Blue); // Profile the RenderScene function

    // Start profiling the entire RenderScene process
    {
        PROFILE_BLOCK("Clear Framebuffer", Yellow);
        Clear();
        // Profiling block ends when the scope ends
    }

    if (scene != m_CurrentScene)
    {
        {
            PROFILE_BLOCK("Initialize New Scene Passes", Yellow);
            m_CurrentScene = scene;
            InitializePassesForScene(scene);
            // Profiling block ends here
        }
    }

    {
        PROFILE_BLOCK("Execute Render Passes", Yellow);
        for (auto& pass : m_RenderPasses)
        {
            pass->Execute(scene);
        }
        // Profiling block ends here
    }
}

void Renderer::Clear(float r, float g, float b, float a) const
{
    PROFILE_FUNCTION(Yellow); // Profile the Clear function
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::InitializePassesForScene(const std::shared_ptr<Scene>& scene)
{
    PROFILE_FUNCTION(Cyan); // Profile the InitializePassesForScene function

    // Start a profiling block for initializing render passes
    {
        PROFILE_BLOCK("Initialize Passes for Scene", Green);
        m_RenderPasses.clear();

        auto framebuffer = CreateFramebufferForScene(scene, m_Width, m_Height);

        {
            PROFILE_BLOCK("Add GeometryPass", Green);
            m_RenderPasses.push_back(std::make_unique<GeometryPass>(framebuffer, scene));
            // Profiling block ends here
        }

        if (scene->GetBGrid())
        {
            {
                PROFILE_BLOCK("Add GridPass", Green);
                m_RenderPasses.push_back(std::make_unique<GridPass>(framebuffer, scene));
                // Profiling block ends here
            }
        }

        //if (scene->GetTerrainHeightMap())
        //{
        //    {
        //        PROFILE_BLOCK("Add TerrainPass", Green);
        //        m_RenderPasses.push_back(std::make_unique<TerrainPass>(framebuffer, scene));
        //        // Profiling block ends here
        //    }
        //}

        if (scene->GetBDebugLights())
        {
            {
                PROFILE_BLOCK("Add DebugLightsPass", Green);
                m_RenderPasses.push_back(std::make_unique<DebugLightsPass>(framebuffer, scene));
                // Profiling block ends here
            }
        }

        {
            PROFILE_BLOCK("Add PostProcessingPass", Green);
            auto ppPass = std::make_unique<PostProcessingPass>(framebuffer, scene);
            auto& effectManager = EffectsManager::GetInstance();
            auto edgeEffect = effectManager.GetEffect(scene->GetPostProcessingEffect());
            ppPass->SetPostProcessingEffect(edgeEffect);
            m_RenderPasses.push_back(std::move(ppPass));
            // Profiling block ends here
        }
        // Profiling block ends here
    }
}

std::shared_ptr<FrameBuffer> Renderer::CreateFramebufferForScene(const std::shared_ptr<Scene>& scene, int width, int height)
{
    PROFILE_FUNCTION(Yellow); // Profile the CreateFramebufferForScene function

    std::vector<FrameBufferTextureAttachment> colorAttachments = {
        { GL_COLOR_ATTACHMENT0, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE }       // Will need more attachments later
    };

    auto framebuffer = std::make_shared<FrameBuffer>(width, height, colorAttachments, true);

    return framebuffer;
}

void Renderer::OnWindowResize(int width, int height)
{
    PROFILE_FUNCTION(Green); // Profile the OnWindowResize function
    m_Width = width;
    m_Height = height;

    if (m_CurrentScene)
    {
        {
            PROFILE_BLOCK("Recreate Framebuffer on Resize", Yellow);
            auto framebuffer = CreateFramebufferForScene(m_CurrentScene, m_Width, m_Height);
            for (auto& pass : m_RenderPasses)
            {
                pass->UpdateFramebuffer(framebuffer);
            }
            // Profiling block ends here
        }
    }

    {
        PROFILE_BLOCK("Handle Effects Manager Resize", Yellow);
        auto& effectManager = EffectsManager::GetInstance();
        effectManager.OnWindowResize(m_Width, m_Height);
        // Profiling block ends here
    }
}