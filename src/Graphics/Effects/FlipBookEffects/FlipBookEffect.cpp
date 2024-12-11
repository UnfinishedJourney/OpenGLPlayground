#include "FlipbookEffect.h"
#include "Utilities/Logger.h"
#include "Resources/ResourceManager.h"
#include "Resources/ShaderManager.h"
#include "GLFW/glfw3.h"

FlipbookEffect::FlipbookEffect(std::shared_ptr<MeshBuffer> quadMeshBuffer)
    : m_QuadMeshBuffer(quadMeshBuffer)
{
    m_Shader = ShaderManager::GetInstance().GetShader("Flipbook");
    if (!m_Shader) {
        Logger::GetLogger()->error("FlipbookEffect: 'Flipbook' shader not found!");
        throw std::runtime_error("Flipbook shader not found");
    }
}

void FlipbookEffect::LoadConfig(const std::string& basePath, const std::string& framesFile, uint32_t totalFrames, float fps, bool loop) {
    m_TotalFrames = totalFrames;
    m_FramesPerSecond = fps;
    m_Loop = loop;

    // For simplicity, assume framesFile is a texture array file or a single file that ResourceManager can load as array.
    // If it's a single sprite-sheet, you'd need a preprocessing step.

    std::string fullPath = basePath + framesFile;
    TextureConfig config;
    config.internalFormat = GL_RGBA8;
    config.generateMips = false;
    config.wrapS = GL_CLAMP_TO_EDGE;
    config.wrapT = GL_CLAMP_TO_EDGE;

    // Load texture array
    // If you actually have multiple frames as separate files, you must build a vector of file paths.
    // If a single tga contains 64 frames in a grid, you must load them into a texture array. This may require custom code.
    // For now, we assume the ResourceManager or the OpenGLTextureArray can handle a single multi-layer file.
    std::vector<std::string> framePaths;
    framePaths.push_back(fullPath); // If this is a single-file array
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