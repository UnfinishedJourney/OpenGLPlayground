#pragma once
#include <memory>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "Graphics/Buffers/MeshBuffer.h"
#include "Graphics/Shaders/BaseShader.h"
#include "Graphics/Textures/OpenGLTextureArray.h"
#include "FlipBookAnimation.h"

class FlipbookEffect {
public:
    FlipbookEffect(std::shared_ptr<Graphics::MeshBuffer> quadMeshBuffer);
    ~FlipbookEffect() = default;

    // Load configuration and frames
    void LoadConfig(const std::string& basePath, const std::string& framesFile, uint32_t totalFrames, uint32_t gridX, uint32_t gridY, float fps, bool loop);

    // Spawn a new animation instance at given position
    void SpawnAnimation(const glm::vec2& position, uint32_t flipbookOffset = 0);

    // Update all animations
    void Update(double currentTime);

    // Render all animations
    void Render();

private:
    std::shared_ptr<Graphics::MeshBuffer> m_QuadMeshBuffer;
    std::shared_ptr<BaseShader> m_Shader;
    std::shared_ptr<OpenGLTextureArray> m_TextureArray;

    std::vector<FlipbookAnimation> m_Animations;

    uint32_t m_TotalFrames = 64;
    uint32_t m_GridX = 8;
    uint32_t m_GridY = 8;
    float m_FramesPerSecond = 30.0f;
    bool m_Loop = false;
};