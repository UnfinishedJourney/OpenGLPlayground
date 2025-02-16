#include "FlipbookEffect.h"
#include "Utilities/Logger.h"
#include "Resources/ResourceManager.h"
#include "Graphics/Shaders/ShaderManager.h"
#include "Scene/Screen.h"
#include "Utilities/Utility.h"
#include <GLFW/glfw3.h>
#include "Graphics/Shaders/Shader.h"

FlipbookEffect::FlipbookEffect(std::shared_ptr<graphics::MeshBuffer> quadMeshBuffer)
    : quadMeshBuffer_(quadMeshBuffer)
{
    shader_ = graphics::ShaderManager::GetInstance().GetShader("flipbook");
    if (!shader_) {
        Logger::GetLogger()->error("FlipbookEffect: 'flipbook' shader not found!");
        throw std::runtime_error("Flipbook shader not found");
    }
}

void FlipbookEffect::LoadConfig(const std::string& basePath, const std::string& framesFile,
    uint32_t totalFrames, uint32_t gridX, uint32_t gridY,
    float fps, bool loop)
{
    totalFrames_ = totalFrames;
    gridX_ = gridX;
    gridY_ = gridY;
    framesPerSecond_ = fps;
    loop_ = loop;

    std::string fullPath = basePath + framesFile;
    graphics::TextureConfig config;
    config.internal_format_ = GL_RGBA8;
    config.generate_mips_ = false;
    config.wrap_s_ = GL_CLAMP_TO_EDGE;
    config.wrap_t_ = GL_CLAMP_TO_EDGE;

    std::vector<std::string> framePaths = { fullPath };

    textureArray_ = std::make_shared<graphics::OpenGLTextureArray>(framePaths, config, totalFrames, gridX, gridY);

    Logger::GetLogger()->info("FlipbookEffect: Loaded flipbook from {} ({} frames).", fullPath, totalFrames);
}

void FlipbookEffect::SpawnAnimation(const glm::vec2& position, uint32_t flipbookOffset) {
    FlipbookAnimation anim;
    anim.position_ = position;
    anim.startTime_ = glfwGetTime();
    anim.flipbookOffset_ = flipbookOffset;
    anim.totalFrames_ = totalFrames_;
    anim.framesPerSecond_ = framesPerSecond_;
    animations_.push_back(anim);
}

void FlipbookEffect::Update(double currentTime) {
    for (auto& anim : animations_) {
        UpdateFlipbook(anim, currentTime, loop_);
    }

    if (!loop_) {
        animations_.erase(
            std::remove_if(animations_.begin(), animations_.end(),
                [&](const FlipbookAnimation& a) { return a.IsFinished(currentTime); }),
            animations_.end()
        );
    }
}

void FlipbookEffect::Render() {
    if (!textureArray_ || !shader_ || !quadMeshBuffer_) {
        return;
    }

    GLCall(glDisable(GL_DEPTH_TEST));
    shader_->Bind();
    textureArray_->Bind(0);
    shader_->SetUniform("texFlipbookArray", 0);

    quadMeshBuffer_->Bind();
    for (auto& anim : animations_) {
        float aspectRatio = static_cast<float>(Screen::width_) / Screen::height_;
        shader_->SetUniform("currentFrame", static_cast<int>(anim.currentFrame_));
        shader_->SetUniform("flipbookPosition", anim.position_);
        shader_->SetUniform("aspectRatio", aspectRatio);
        GLCall(glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(quadMeshBuffer_->GetIndexCount()), GL_UNSIGNED_INT, nullptr));
    }
    quadMeshBuffer_->Unbind();

    shader_->Unbind();
    GLCall(glEnable(GL_DEPTH_TEST));
}