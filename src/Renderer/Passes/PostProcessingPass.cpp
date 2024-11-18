#include "PostProcessingPass.h"
#include "Resources/ResourceManager.h"
#include "Utilities/Logger.h"
#include <glad/glad.h>

PostProcessingPass::PostProcessingPass(std::shared_ptr<FrameBuffer> framebuffer, const std::shared_ptr<Scene>& scene)
    : m_Framebuffer(framebuffer)
{
    SetupFullscreenQuad();
}

PostProcessingPass::~PostProcessingPass()
{
    if (m_FullscreenQuadVAO)
        glDeleteVertexArrays(1, &m_FullscreenQuadVAO);
    if (m_FullscreenQuadVBO)
        glDeleteBuffers(1, &m_FullscreenQuadVBO);
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

void PostProcessingPass::SetupFullscreenQuad()
{
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,   0.0f, 1.0f,
        -1.0f, -1.0f,   0.0f, 0.0f,
         1.0f, -1.0f,   1.0f, 0.0f,

        -1.0f,  1.0f,   0.0f, 1.0f,
         1.0f, -1.0f,   1.0f, 0.0f,
         1.0f,  1.0f,   1.0f, 1.0f
    };

    glGenVertexArrays(1, &m_FullscreenQuadVAO);
    glGenBuffers(1, &m_FullscreenQuadVBO);
    glBindVertexArray(m_FullscreenQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_FullscreenQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    // TexCoord attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);

    Logger::GetLogger()->info("Fullscreen Quad VAO and VBO set up.");
}

void PostProcessingPass::UpdateFramebuffer(std::shared_ptr<FrameBuffer> framebuffer)
{
    m_Framebuffer = framebuffer;
}