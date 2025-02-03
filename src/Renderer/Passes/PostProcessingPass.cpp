#include "PostProcessingPass.h"
#include "Utilities/Logger.h"
#include <glad/glad.h>

PostProcessingPass::PostProcessingPass(std::shared_ptr<FrameBuffer> /*ignored*/,
    const std::shared_ptr<Scene>& scene)
{
    // No need to store that "ignored" FBO. We'll rely on m_SourceFBO.
    // Possibly store scene references if needed.
}

PostProcessingPass::~PostProcessingPass()
{
}

void PostProcessingPass::SetPostProcessingEffect(const std::shared_ptr<PostProcessingEffect>& effect)
{
    m_Effect = effect;
}

void PostProcessingPass::SetSourceFramebuffer(std::shared_ptr<FrameBuffer> sourceFBO)
{
    m_SourceFBO = sourceFBO;
}

void PostProcessingPass::Execute(const std::shared_ptr<Scene>& scene)
{
    if (!m_SourceFBO) {
        Logger::GetLogger()->error("PostProcessingPass: No source FBO set!");
        return;
    }
    if (!m_Effect) {
        Logger::GetLogger()->error("PostProcessingPass: No PostProcessingEffect set!");
        return;
    }

    // 1) Grab the resolved texture from the single-sample FBO
    GLuint sceneTextureID = m_SourceFBO->GetTexture(GL_COLOR_ATTACHMENT0);
    if (sceneTextureID == 0) {
        Logger::GetLogger()->error("PostProcessingPass: Invalid texture ID (0).");
        return;
    }

    // 2) The effect’s "Apply" might draw to the default FBO (ID=0) or another FBO
    //    depending on your setup.
    //    Example: output to the main screen:
    m_Effect->Apply(sceneTextureID, /*outputFramebuffer=*/0);
}

void PostProcessingPass::UpdateFramebuffer(std::shared_ptr<FrameBuffer> framebuffer)
{
    // If we want to change the source FBO, we do so here.
    // But typically we do SetSourceFramebuffer(...) for clarity.
    SetSourceFramebuffer(framebuffer);
}