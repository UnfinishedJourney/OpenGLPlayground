#include "PostProcessingPass.h"
#include "Resources/ResourceManager.h"
#include "Utilities/Logger.h"
#include <glad/glad.h>

PostProcessingPass::PostProcessingPass(GLuint fullscreenQuadVAO, std::shared_ptr<FrameBuffer> framebuffer, const std::shared_ptr<Scene>& scene)
    : m_FullscreenQuadVAO(fullscreenQuadVAO), m_Framebuffer(framebuffer)
{
    // Initialize scene-specific resources if needed
}

void PostProcessingPass::Execute(const std::shared_ptr<Scene>& scene)
{
    glDisable(GL_DEPTH_TEST);
    GLuint sceneTextureID = m_Framebuffer->GetTexture(GL_COLOR_ATTACHMENT0);

    if (sceneTextureID == 0)
    {
        Logger::GetLogger()->error("No valid texture in framebuffer for PostProcessingPass.");
        return;
    }

    auto postProcessingShader = ResourceManager::GetInstance().GetShader(scene->GetPostProcessingShaderName());
    if (!postProcessingShader)
    {
        Logger::GetLogger()->error("Post-processing shader '{}' not found!", scene->GetPostProcessingShaderName());
        return;
    }

    ResourceManager::GetInstance().BindShader(scene->GetPostProcessingShaderName());

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Render to the screen

    glClear(GL_COLOR_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneTextureID);
    ResourceManager::GetInstance().SetUniform("u_SceneTexture", 0);

    glBindVertexArray(m_FullscreenQuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);

    Logger::GetLogger()->debug("Post-processing pass executed.");
}

void PostProcessingPass::UpdateFramebuffer(std::shared_ptr<FrameBuffer> framebuffer)
{
    m_Framebuffer = framebuffer;
}