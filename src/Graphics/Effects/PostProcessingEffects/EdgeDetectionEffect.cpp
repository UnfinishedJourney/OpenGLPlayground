#include "EdgeDetectionEffect.h"
#include "Resources/ResourceManager.h"
#include "Utilities/Logger.h"
#include <glad/glad.h>

EdgeDetectionEffect::EdgeDetectionEffect(std::shared_ptr<graphics::MeshBuffer> quad, int width, int height)
    : PostProcessingEffect(quad, width, height),
    edgeThreshold_(0.05f) // default threshold
{
    auto& shaderManager = graphics::ShaderManager::GetInstance();
    m_Shader = shaderManager.GetShader("edgeDetection");
    if (!m_Shader) {
        Logger::GetLogger()->error("EdgeDetection shader not found!");
    }

    if (width > 0 && height > 0) {
        texelSize_ = glm::vec2(1.0f / static_cast<float>(width), 1.0f / static_cast<float>(height));
    }
    else {
        texelSize_ = glm::vec2(0.0f);
        Logger::GetLogger()->warn("Invalid window dimensions for EdgeDetectionEffect.");
    }
}

void EdgeDetectionEffect::Apply(GLuint inputTexture, GLuint outputFramebuffer) {
    glBindFramebuffer(GL_FRAMEBUFFER, outputFramebuffer);
    glViewport(0, 0, m_Width, m_Height);
    glDisable(GL_DEPTH_TEST);

    if (!m_Shader || !m_FullscreenQuadMeshBuffer) {
        Logger::GetLogger()->error("EdgeDetectionEffect not properly initialized.");
        return;
    }

    m_Shader->Bind();
    m_Shader->SetUniform("EdgeThreshold", edgeThreshold_);
    m_Shader->SetUniform("texelSize", texelSize_);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTexture);
    m_Shader->SetUniform("u_InputTexture", 0);

    m_FullscreenQuadMeshBuffer->Bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    m_FullscreenQuadMeshBuffer->Unbind();

    glEnable(GL_DEPTH_TEST);
}

void EdgeDetectionEffect::OnWindowResize(int width, int height) {
    m_Width = width;
    m_Height = height;

    if (m_Width > 0 && m_Height > 0) {
        texelSize_ = glm::vec2(1.0f / static_cast<float>(m_Width), 1.0f / static_cast<float>(m_Height));
    }
    else {
        texelSize_ = glm::vec2(0.0f);
        Logger::GetLogger()->warn("Invalid window dimensions received in OnWindowResize.");
    }
}

void EdgeDetectionEffect::SetEdgeThreshold(float threshold) {
    if (threshold >= 0.0f && threshold <= 1.0f) {
        edgeThreshold_ = threshold;
        Logger::GetLogger()->info("EdgeThreshold set to {}", edgeThreshold_);
    }
    else {
        Logger::GetLogger()->warn("Attempted to set invalid EdgeThreshold value: {}", threshold);
    }
}

void EdgeDetectionEffect::SetParameters(const std::unordered_map<std::string, EffectParameter>& params) {
    for (const auto& [key, value] : params) {
        if (key == "EdgeThreshold") {
            if (auto threshold = std::get_if<float>(&value)) {
                SetEdgeThreshold(*threshold);
            }
            else {
                Logger::GetLogger()->warn("Invalid type for 'EdgeThreshold' parameter in EdgeDetectionEffect.");
            }
        }
        else {
            Logger::GetLogger()->warn("Unknown parameter '{}' for EdgeDetectionEffect.", key);
        }
    }
}
