#include "PostProcessingPass.h"
#include "Utilities/Logger.h"
#include <glad/glad.h>

PostProcessingPass::PostProcessingPass(std::shared_ptr<graphics::FrameBuffer> /*ignored*/,
    const std::shared_ptr<Scene::Scene>& scene)
{
    // Not storing the ignored FBO; use sourceFBO_ instead.
}

PostProcessingPass::~PostProcessingPass() {
    // Resources released by smart pointers.
}

void PostProcessingPass::SetPostProcessingEffect(const std::shared_ptr<PostProcessingEffect>& effect) {
    effect_ = effect;
}

void PostProcessingPass::SetSourceFramebuffer(std::shared_ptr<graphics::FrameBuffer> sourceFBO) {
    sourceFBO_ = sourceFBO;
}

void PostProcessingPass::Execute(const std::shared_ptr<Scene::Scene>& scene) {
    if (!sourceFBO_) {
        Logger::GetLogger()->error("PostProcessingPass: No source FBO set!");
        return;
    }
    if (!effect_) {
        Logger::GetLogger()->error("PostProcessingPass: No PostProcessingEffect set!");
        return;
    }

    GLuint sceneTextureID = sourceFBO_->GetTexture(GL_COLOR_ATTACHMENT0);
    if (sceneTextureID == 0) {
        Logger::GetLogger()->error("PostProcessingPass: Invalid texture ID (0).");
        return;
    }

    // Apply the effect; output to the default framebuffer (0) in this example.
    effect_->Apply(sceneTextureID, 0);
}

void PostProcessingPass::UpdateFramebuffer(std::shared_ptr<graphics::FrameBuffer> framebuffer) {
    SetSourceFramebuffer(framebuffer);
}