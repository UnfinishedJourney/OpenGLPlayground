#include "Renderer.h"
#include <glad/glad.h>
#include <stdexcept>
#include <algorithm>

// Include example passes.
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
    : width_(800), height_(600)
{
    // Global GL state can be set here if needed.
    // e.g., glEnable(GL_DEPTH_TEST);
}

Renderer::~Renderer() = default;

void Renderer::Initialize(int width, int height) {
    width_ = width;
    height_ = height;
    // Optionally, set additional global GL states:
    // glEnable(GL_DEPTH_TEST);
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_BACK);
}

void Renderer::RenderScene(const std::shared_ptr<Scene::Scene>& scene) {
    PROFILE_FUNCTION(Blue);

    // If the scene has changed, reinitialize passes and FBOs.
    if (scene != currentScene_) {
        PROFILE_BLOCK("Initialize New Scene Passes", Yellow);
        currentScene_ = scene;
        InitializePassesForScene(scene);
    }

    // 1) Clear the default framebuffer.
    {
        PROFILE_BLOCK("Clear Screen", Yellow);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        Clear(0.3f, 0.2f, 0.8f, 1.0f);
    }

    // 2) Execute shadow passes.
    for (auto& pass : shadowPasses_) {
        pass->Execute(scene);
    }

    // 3) Render geometry into the MSAA FBO.
    {
        PROFILE_BLOCK("Render into MSAA FBO", Cyan);
        if (msaaFBO_) {
            msaaFBO_->Bind();
            Clear(); // Clear color and depth.
        }
        for (auto& pass : geometryPasses_) {
            pass->Execute(scene);
        }
        if (msaaFBO_) {
            msaaFBO_->Unbind();
        }
    }

    // 4) Resolve MSAA FBO to the single-sample FBO.
    {
        PROFILE_BLOCK("Resolve MSAA FBO", Green);
        if (msaaFBO_ && resolvedFBO_) {
            msaaFBO_->BlitTo(*resolvedFBO_, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        }
    }

    // 5) Execute post-processing passes.
    {
        PROFILE_BLOCK("Post-Processing", Magenta);
        for (auto& pass : postProcessingPasses_) {
            pass->Execute(scene);
        }
    }
}

void Renderer::Clear(float r, float g, float b, float a) const {
    PROFILE_FUNCTION(Yellow);
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::OnWindowResize(int width, int height) {
    PROFILE_FUNCTION(Green);
    width_ = width;
    height_ = height;

    // Recreate FBOs and update passes if a scene is loaded.
    if (currentScene_) {
        PROFILE_BLOCK("Recreate FBO on Resize", Yellow);
        CreateFramebuffersForScene(currentScene_, width, height);

        for (auto& pass : geometryPasses_) {
            pass->UpdateFramebuffer(msaaFBO_);
        }
        for (auto& pass : postProcessingPasses_) {
            pass->SetSourceFramebuffer(resolvedFBO_);
        }
    }

    // Inform the EffectsManager of the new size.
    EffectsManager::GetInstance().OnWindowResize(width, height);
}

void Renderer::InitializePassesForScene(const std::shared_ptr<Scene::Scene>& scene) {
    PROFILE_FUNCTION(Cyan);

    // Clear old passes.
    geometryPasses_.clear();
    postProcessingPasses_.clear();

    // Recreate FBOs.
    CreateFramebuffersForScene(scene, width_, height_);

    if (scene->GetShowShadows()) {
        shadowPasses_.push_back(std::make_unique<ShadowPass>(scene, 2 * 2048));
    }
    if (scene->GetSkyboxEnabled()) {
        geometryPasses_.push_back(std::make_unique<SkyBoxPass>(msaaFBO_, scene));
    }
    geometryPasses_.push_back(std::make_unique<GeometryPass>(msaaFBO_, scene));
    if (scene->GetShowGrid()) {
        geometryPasses_.push_back(std::make_unique<GridPass>(msaaFBO_, scene));
    }
    if (scene->GetShowDebugLights()) {
        geometryPasses_.push_back(std::make_unique<DebugLightsPass>(msaaFBO_, scene));
    }
    {
        auto ppPass = std::make_unique<PostProcessingPass>(resolvedFBO_, scene);
        auto effect = EffectsManager::GetInstance().GetEffect(scene->GetPostProcessingEffect());
        ppPass->SetPostProcessingEffect(effect);
        ppPass->SetSourceFramebuffer(resolvedFBO_);
        postProcessingPasses_.push_back(std::move(ppPass));
    }
}

void Renderer::CreateFramebuffersForScene(const std::shared_ptr<Scene::Scene>& scene,
    int width, int height)
{
    PROFILE_FUNCTION(Yellow);

    // MSAA FBO: using 4 samples.
    int samples = 4;
    std::vector<graphics::FrameBufferTextureAttachment> msaaAttachments = {
        { GL_COLOR_ATTACHMENT0, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE }
    };
    msaaFBO_ = std::make_shared<graphics::FrameBuffer>(width, height, msaaAttachments,
        /*hasDepth=*/true, samples);

    // Single-sample FBO (resolved).
    std::vector<graphics::FrameBufferTextureAttachment> resolvedAttachments = {
        { GL_COLOR_ATTACHMENT0, GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE }
    };
    resolvedFBO_ = std::make_shared<graphics::FrameBuffer>(width, height, resolvedAttachments,
        /*hasDepth=*/false, 1);
    glEnable(GL_FRAMEBUFFER_SRGB);
    Logger::GetLogger()->info("Renderer: Created {}x{} MSAA FBO ({} samples) + resolved FBO (1 sample).",
        width, height, samples);
}