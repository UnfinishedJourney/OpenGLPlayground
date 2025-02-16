#pragma once
#include <memory>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "Graphics/Buffers/MeshBuffer.h"
#include "Graphics/Shaders/BaseShader.h"
#include "Graphics/Textures/OpenGLTextureArray.h"
#include "FlipbookAnimation.h"

class FlipbookEffect {
public:
    explicit FlipbookEffect(std::shared_ptr<graphics::MeshBuffer> quadMeshBuffer);
    ~FlipbookEffect() = default;

    void LoadConfig(const std::string& basePath, const std::string& framesFile,
        uint32_t totalFrames, uint32_t gridX, uint32_t gridY, float fps, bool loop);
    void SpawnAnimation(const glm::vec2& position, uint32_t flipbookOffset = 0);
    void Update(double currentTime);
    void Render();

private:
    std::shared_ptr<graphics::MeshBuffer> quadMeshBuffer_;
    std::shared_ptr<graphics::BaseShader> shader_;
    std::shared_ptr<graphics::OpenGLTextureArray> textureArray_;

    std::vector<FlipbookAnimation> animations_;

    uint32_t totalFrames_ = 64;
    uint32_t gridX_ = 8;
    uint32_t gridY_ = 8;
    float framesPerSecond_ = 30.0f;
    bool loop_ = false;
};