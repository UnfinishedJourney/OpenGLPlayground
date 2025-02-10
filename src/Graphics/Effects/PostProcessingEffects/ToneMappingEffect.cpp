#include "ToneMappingEffect.h"
#include "Utilities/Logger.h"
#include "Resources/ShaderManager.h"
#include <glad/glad.h>
#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

ToneMappingEffect::ToneMappingEffect(std::shared_ptr<Graphics::MeshBuffer> quad, int width, int height)
    : PostProcessingEffect(quad, width, height),
    m_Exposure(0.18f),  // Default exposure value.
    m_LWhite(1.0f)      // Default white-point.
{
    // Load the tone mapping shader from your shader manager.
    auto& shaderManager = Graphics::ShaderManager::GetInstance();
    m_Shader = shaderManager.GetShader("toneMapping");
    if (!m_Shader) {
        Logger::GetLogger()->error("ToneMappingEffect: toneMapping shader not found!");
    }
}

ToneMappingEffect::~ToneMappingEffect() {
    // Nothing extra to free here.
}

float ToneMappingEffect::ComputeLogAverageLuminance(GLuint hdrTexture) {
    // Bind the HDR texture and read back its pixel data.
    glBindTexture(GL_TEXTURE_2D, hdrTexture);
    std::vector<float> hdrData(m_Width * m_Height * 4); // RGBA32F
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, hdrData.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    // Compute the log-average luminance.
    double logSum = 0.0;
    const double delta = 0.0001;  // Prevent log(0)
    int totalPixels = m_Width * m_Height;
    for (int i = 0; i < totalPixels; i++) {
        float r = hdrData[i * 4 + 0];
        float g = hdrData[i * 4 + 1];
        float b = hdrData[i * 4 + 2];
        // Standard luminance calculation.
        float Lw = 0.2126f * r + 0.7152f * g + 0.0722f * b;
        logSum += std::log(delta + Lw);
    }
    float logAvg = std::exp(logSum / totalPixels);
    return logAvg;
}

void ToneMappingEffect::Apply(GLuint inputTexture, GLuint outputFramebuffer) {
    // Step 1: Compute the log–average luminance of the HDR image on the CPU.
    float logAvg = ComputeLogAverageLuminance(inputTexture);

    // Step 2: Render the final LDR image using the tone mapping shader.
    glBindFramebuffer(GL_FRAMEBUFFER, outputFramebuffer);
    glViewport(0, 0, m_Width, m_Height);
    glDisable(GL_DEPTH_TEST);

    if (!m_Shader || !m_FullscreenQuadMeshBuffer) {
        Logger::GetLogger()->error("ToneMappingEffect: Shader or fullscreen quad not available.");
        return;
    }
    m_Shader->Bind();

    // Pass the tone mapping parameters to the shader.
    m_Shader->SetUniform("u_Exposure", m_Exposure);
    m_Shader->SetUniform("u_LWhite", m_LWhite);
    m_Shader->SetUniform("u_logAvg", logAvg);

    // Bind the HDR texture to texture unit 0.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTexture);
    m_Shader->SetUniform("u_HDRTexture", 0);

    // Render the fullscreen quad.
    m_FullscreenQuadMeshBuffer->Bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    m_FullscreenQuadMeshBuffer->Unbind();

    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ToneMappingEffect::OnWindowResize(int width, int height) {
    m_Width = width;
    m_Height = height;
    // (If your fullscreen quad or internal textures need to be recreated, do it here.)
}

void ToneMappingEffect::SetParameters(const std::unordered_map<std::string, EffectParameter>& params) {
    // Expecting parameters "Exposure" and "LWhite".
    for (const auto& [key, value] : params) {
        if (key == "Exposure") {
            if (auto pVal = std::get_if<float>(&value)) {
                m_Exposure = *pVal;
            }
            else {
                Logger::GetLogger()->warn("ToneMappingEffect: Invalid type for parameter 'Exposure'.");
            }
        }
        else if (key == "LWhite") {
            if (auto pVal = std::get_if<float>(&value)) {
                m_LWhite = *pVal;
            }
            else {
                Logger::GetLogger()->warn("ToneMappingEffect: Invalid type for parameter 'LWhite'.");
            }
        }
        else {
            Logger::GetLogger()->warn("ToneMappingEffect: Unknown parameter '{}'.", key);
        }
    }
}