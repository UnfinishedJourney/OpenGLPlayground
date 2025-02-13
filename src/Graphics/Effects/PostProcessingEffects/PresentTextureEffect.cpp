#include "PresentTextureEffect.h"
#include "Resources/ResourceManager.h"
#include "Utilities/Logger.h"
#include <glad/glad.h>

PresentTextureEffect::PresentTextureEffect(std::shared_ptr<graphics::MeshBuffer> quad, int width, int height)
    : PostProcessingEffect(quad, width, height)
{

    auto& shaderManager = graphics::ShaderManager::GetInstance();
    m_Shader = shaderManager.GetShader("presentTexture");
    if (!m_Shader) {
        Logger::GetLogger()->error("Present texture shader not found!");
    }

    auto& textureManager = graphics::TextureManager::GetInstance();

    m_Texture = textureManager.GetTexture("brdfLUT");
    //m_Texture = textureManager.GetTexture2D("cuteDog");
}

void PresentTextureEffect::SetParameters(const std::unordered_map<std::string, EffectParameter>& params)
{
    for (const auto& [key, value] : params) {
        if (key == "texture") {
            if (auto tex = std::get_if<std::shared_ptr<graphics::ITexture>>(&value)) {
                SetTexture(*tex);
            }
            else {
                Logger::GetLogger()->warn("Invalid type for 'texture' parameter in PresentTextureEffect.");
            }
        }
        // Handle other parameters if necessary
    }
}

void PresentTextureEffect::SetTexture(std::shared_ptr<graphics::ITexture> texture)
{
    if (texture) {
        m_Texture = texture;
    }
    else {
        Logger::GetLogger()->warn("Attempted to set a null texture in PresentTextureEffect.");
    }
}

void PresentTextureEffect::Apply(GLuint inputTexture, GLuint outputFramebuffer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, outputFramebuffer);
    glViewport(0, 0, m_Width, m_Height);
    glDisable(GL_DEPTH_TEST);

    if (!m_Shader || !m_FullscreenQuadMeshBuffer) {
        Logger::GetLogger()->error("PresentTextureEffect not properly initialized.");
        return;
    }

    auto& resourceManager = ResourceManager::GetInstance();
    if (!m_Shader) {
        Logger::GetLogger()->error("Present texture shader not found!");
        return;
    }

    m_Shader->Bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTexture);

    m_Texture->Bind(1);
    m_FullscreenQuadMeshBuffer->Bind();


    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    m_FullscreenQuadMeshBuffer->Unbind();

    glEnable(GL_DEPTH_TEST);
}

void PresentTextureEffect::OnWindowResize(int width, int height)
{
    m_Width = width;
    m_Height = height;
}