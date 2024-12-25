#include "NoPostProcessingEffect.h"
#include "Resources/ResourceManager.h"
#include "Utilities/Logger.h"
#include <glad/glad.h>

NoPostProcessingEffect::NoPostProcessingEffect(std::shared_ptr<MeshBuffer> quad, int width, int height)
    : PostProcessingEffect(quad, width, height)
{

    auto& shaderManager = ShaderManager::GetInstance();
    m_Shader = shaderManager.GetShader("noPostProcessing");
    if (!m_Shader) {
        Logger::GetLogger()->error("No postprocessing shader not found!");
    }

}


void NoPostProcessingEffect::Apply(GLuint inputTexture, GLuint outputFramebuffer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, outputFramebuffer);
    glViewport(0, 0, m_Width, m_Height);
    glDisable(GL_DEPTH_TEST);

    if (!m_Shader || !m_FullscreenQuadMeshBuffer) {
        Logger::GetLogger()->error("NoPostProcessingEffect not properly initialized.");
        return;
    }

    auto& resourceManager = ResourceManager::GetInstance();
    if (!m_Shader) {
        Logger::GetLogger()->error("No postprocessing shader not found!");
        return;
    }

    m_Shader->Bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTexture);

    m_FullscreenQuadMeshBuffer->Bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    m_FullscreenQuadMeshBuffer->Unbind();

    glEnable(GL_DEPTH_TEST);
}

void NoPostProcessingEffect::OnWindowResize(int width, int height)
{
    m_Width = width;
    m_Height = height;
}