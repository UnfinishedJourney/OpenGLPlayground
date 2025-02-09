#include "FlipbookEffect.h"
#include "Utilities/Logger.h"
#include "Resources/ResourceManager.h"
#include "Resources/ShaderManager.h"
#include "Scene/Screen.h"
#include "Utilities/Utility.h"
#include <GLFW/glfw3.h>
#include "Graphics/Shaders/Shader.h"

FlipbookEffect::FlipbookEffect(std::shared_ptr<Graphics::MeshBuffer> quadMeshBuffer)
    : m_QuadMeshBuffer(quadMeshBuffer)
{
    m_Shader = ShaderManager::GetInstance().GetShader("flipbook");
    if (!m_Shader) {
        Logger::GetLogger()->error("FlipbookEffect: 'Flipbook' shader not found!");
        throw std::runtime_error("Flipbook shader not found");
    }
}

void FlipbookEffect::LoadConfig(const std::string& basePath, const std::string& framesFile, uint32_t totalFrames, uint32_t gridX, uint32_t gridY, float fps, bool loop) {
    m_TotalFrames = totalFrames;
    m_GridX = gridX;
    m_GridY = gridY;
    m_FramesPerSecond = fps;
    m_Loop = loop;

    std::string fullPath = basePath + framesFile;
    Graphics::TextureConfig config;
    config.internalFormat = GL_RGBA8;
    config.generateMips = false;
    config.wrapS = GL_CLAMP_TO_EDGE;
    config.wrapT = GL_CLAMP_TO_EDGE;
    //config.minFilter = GL_LINEAR;
    //config.magFilter = GL_LINEAR;

    std::vector<std::string> framePaths;
    framePaths.push_back(fullPath);

    m_TextureArray = std::make_shared<Graphics::OpenGLTextureArray>(framePaths, config, totalFrames, gridX, gridY);

    Logger::GetLogger()->info("FlipbookEffect: Loaded flipbook from {} ({} frames).", fullPath, totalFrames);
}

void FlipbookEffect::SpawnAnimation(const glm::vec2& position, uint32_t flipbookOffset) {
    FlipbookAnimation anim;
    anim.position = position;
    anim.startTime = glfwGetTime();
    anim.flipbookOffset = flipbookOffset;
    anim.totalFrames = m_TotalFrames;
    anim.framesPerSecond = m_FramesPerSecond;
    m_Animations.push_back(anim);
}

void FlipbookEffect::Update(double currentTime) {
    for (auto& anim : m_Animations) {
        UpdateFlipbook(anim, currentTime, m_Loop);
    }

    if (!m_Loop) {
        m_Animations.erase(
            std::remove_if(m_Animations.begin(), m_Animations.end(), [&](const FlipbookAnimation& a) {
                return a.IsFinished(currentTime);
                }),
            m_Animations.end()
        );
    }
}

void FlipbookEffect::Render() {
    if (!m_TextureArray || !m_Shader || !m_QuadMeshBuffer) {
        return;
    }

    GLCall(glDisable(GL_DEPTH_TEST));
    m_Shader->Bind();
    m_TextureArray->Bind(0);
    m_Shader->SetUniform("texFlipbookArray", 0);

    m_QuadMeshBuffer->Bind();
    for (auto& anim : m_Animations) {
        float asR = float(Screen::s_Width) / Screen::s_Height;
        m_Shader->SetUniform("currentFrame", (int)anim.currentFrame);
        m_Shader->SetUniform("flipbookPosition", anim.position);
        m_Shader->SetUniform("aspectRatio", asR);
        GLCall(glDrawElements(GL_TRIANGLES, (GLsizei)m_QuadMeshBuffer->GetIndexCount(), GL_UNSIGNED_INT, nullptr));
    }
    m_QuadMeshBuffer->Unbind();

    m_Shader->Unbind();
    GLCall(glEnable(GL_DEPTH_TEST));
}