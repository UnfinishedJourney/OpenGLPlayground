#pragma once
#include <memory>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "FlipbookAnimation.h"
#include "Graphics/Buffers/MeshBuffer.h"
#include "Graphics/Shaders/BaseShader.h"
#include "Graphics/Textures/OpenGLTextureArray.h"

class FlipbookEffect {
public:
    FlipbookEffect(std::shared_ptr<MeshBuffer> quadMeshBuffer);
    ~FlipbookEffect() = default;

    // Load parameters from config (called by EffectsManager)
    void LoadConfig(const std::string& basePath, const std::string& framesFile, uint32_t totalFrames, float fps, bool loop);

    // Spawn a new animation instance
    void SpawnAnimation(const glm::vec2& position, uint32_t flipbookOffset = 0);

    // Update all animations
    void Update(double currentTime);

    // Render all animations
    void Render();

private:
    std::shared_ptr<MeshBuffer> m_QuadMeshBuffer;
    std::shared_ptr<BaseShader> m_Shader;
    std::shared_ptr<OpenGLTextureArray> m_TextureArray;

    std::vector<FlipbookAnimation> m_Animations;

    uint32_t m_TotalFrames = 64;
    float m_FramesPerSecond = 30.0f;
    bool m_Loop = false;
};
