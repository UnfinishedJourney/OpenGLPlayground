#include "PostProcessingPass.h"
#include "Resources/ResourceManager.h"
#include "Utilities/Logger.h"

PostProcessingPass::PostProcessingPass(std::shared_ptr<FrameBuffer> framebuffer, const std::shared_ptr<Scene>& scene)
    : m_Framebuffer(framebuffer)
{
    // Initialize with a default effect or set it later
}

PostProcessingPass::~PostProcessingPass()
{
    // No resources to clean up
}

void PostProcessingPass::SetPostProcessingEffect(const std::shared_ptr<PostProcessingEffect>& effect)
{
    m_Effect = effect;
    if (m_Effect) {
        m_Effect->Initialize();
    }
}

void PostProcessingPass::Execute(const std::shared_ptr<Scene>& scene)
{
    GLuint sceneTextureID = m_Framebuffer->GetTexture(GL_COLOR_ATTACHMENT0);

    if (sceneTextureID == 0)
    {
        Logger::GetLogger()->error("No valid texture in framebuffer for PostProcessingPass.");
        return;
    }

    if (!m_Effect) {
        Logger::GetLogger()->error("No PostProcessingEffect set in PostProcessingPass.");
        return;
    }

    m_Effect->Apply(sceneTextureID, 0); // Output to default framebuffer (screen)
}

void PostProcessingPass::UpdateFramebuffer(std::shared_ptr<FrameBuffer> framebuffer)
{
    m_Framebuffer = framebuffer;
}