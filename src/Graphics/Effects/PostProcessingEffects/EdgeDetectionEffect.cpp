#include "EdgeDetectionEffect.h"
#include "Resources/ResourceManager.h"
#include "Utilities/Logger.h"
#include <glad/glad.h>

EdgeDetectionEffect::EdgeDetectionEffect(std::shared_ptr<MeshBuffer> quad, int width, int height)
    : PostProcessingEffect(quad, width, height), m_EdgeThreshold(0.05f)
{

    auto& resourceManager = ResourceManager::GetInstance();
    m_Shader = resourceManager.GetShader("edgeDetection");
    if (!m_Shader) {
        Logger::GetLogger()->error("EdgeDetection shader not found!");
    }

    m_TexelSize = glm::vec2(1.0f / m_Width, 1.0f / m_Height);
}

void EdgeDetectionEffect::SetParameters(const std::unordered_map<std::string, float>& params)
{
    auto it = params.find("EdgeThreshold");
    if (it != params.end()) {
        SetEdgeThreshold(it->second);
    }
}

void EdgeDetectionEffect::Apply(GLuint inputTexture, GLuint outputFramebuffer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, outputFramebuffer);
    glViewport(0, 0, m_Width, m_Height);
    glDisable(GL_DEPTH_TEST);

    if (!m_Shader || !m_FullscreenQuadMeshBuffer) {
        Logger::GetLogger()->error("EdgeDetectionEffect not properly initialized.");
        return;
    }

    auto& resourceManager = ResourceManager::GetInstance();
    resourceManager.BindShader("edgeDetection");
    resourceManager.SetUniform("EdgeThreshold", m_EdgeThreshold);
    resourceManager.SetUniform("texelSize", m_TexelSize);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTexture);

    m_FullscreenQuadMeshBuffer->Bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    m_FullscreenQuadMeshBuffer->Unbind();

    glEnable(GL_DEPTH_TEST);
}

void EdgeDetectionEffect::OnWindowResize(int width, int height)
{
    m_Width = width;
    m_Height = height;
    m_TexelSize = glm::vec2(1.0f / m_Width, 1.0f / m_Height);
}

void EdgeDetectionEffect::SetEdgeThreshold(float threshold)
{
    m_EdgeThreshold = threshold;
}