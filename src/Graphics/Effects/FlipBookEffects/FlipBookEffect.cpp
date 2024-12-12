#include "FlipbookEffect.h"
#include "Utilities/Logger.h"
#include "Resources/ResourceManager.h"
#include "Resources/ShaderManager.h"
#include "Utilities/Utility.h"
#include <GLFW/glfw3.h>

FlipbookEffect::FlipbookEffect(std::shared_ptr<MeshBuffer> quadMeshBuffer)
    : m_QuadMeshBuffer(quadMeshBuffer)
{
    m_Shader = ShaderManager::GetInstance().GetShader("flipbook");
    if (!m_Shader) {
        Logger::GetLogger()->error("FlipbookEffect: 'Flipbook' shader not found!");
        throw std::runtime_error("Flipbook shader not found");
    }
}

void FlipbookEffect::LoadConfig(const std::string& basePath, const std::string& framesFile, uint32_t totalFrames, float fps, bool loop) {
    m_TotalFrames = totalFrames;
    m_FramesPerSecond = fps;
    m_Loop = loop;

    // Assume framesFile is a single texture array file or a single-file sprite sheet
    // If it's a single sprite sheet that needs slicing, you'd need additional code
    // For now, assume we can load it as a single-layer texture array with one entry
    // or that OpenGLTextureArray can handle single-file arrays.

    std::string fullPath = basePath + framesFile;
    TextureConfig config;
    config.internalFormat = GL_RGBA8;
    config.generateMips = false;
    config.wrapS = GL_CLAMP_TO_EDGE;
    config.wrapT = GL_CLAMP_TO_EDGE;

    std::vector<std::string> framePaths;
    framePaths.push_back(fullPath);
    m_TextureArray = std::make_shared<OpenGLTextureArray>(framePaths, config);

    Logger::GetLogger()->info("FlipbookEffect: Loaded flipbook from {}", fullPath);
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
        m_Shader->SetUniform("currentFrame", (int)anim.currentFrame);
        m_Shader->SetUniform("flipbookPosition", anim.position);
        GLCall(glDrawElements(GL_TRIANGLES, (GLsizei)m_QuadMeshBuffer->GetIndexCount(), GL_UNSIGNED_INT, nullptr));
    }
    m_QuadMeshBuffer->Unbind();

    m_Shader->Unbind();
    GLCall(glEnable(GL_DEPTH_TEST));
}