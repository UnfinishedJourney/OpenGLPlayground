#pragma once
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "Graphics/Effects/PostProcessingEffects/PostProcessingEffect.h"
#include "Graphics/Effects/PostProcessingEffects/PostProcessingEffectType.h"
#include "FlipBookEffects/FlipbookEffect.h"

struct FlipbookEffectConfig {
    std::string effectType;
    std::string framesFile;
    uint32_t totalFrames;
    uint32_t gridX;
    uint32_t gridY;
    float framesPerSecond;
    bool loop;
    std::string basePath;
};

class EffectsManager {
public:
    static EffectsManager& GetInstance();

    std::shared_ptr<PostProcessingEffect> GetEffect(PostProcessingEffectType effectType);
    void SetEffectParameters(PostProcessingEffectType effectType, const std::unordered_map<std::string, EffectParameter>& params);

    void OnWindowResize(int width, int height);

    // Flipbook Effect
    std::shared_ptr<FlipbookEffect> GetFlipbookEffect(const std::string& name);
    void LoadFlipbookEffect(const std::string& name);

private:
    EffectsManager();
    ~EffectsManager();

    EffectsManager(const EffectsManager&) = delete;
    EffectsManager& operator=(const EffectsManager&) = delete;

    FlipbookEffectConfig LoadFlipbookConfig(const std::string& name);

    void SetupFullscreenQuad();
    void SetupFlipbookQuad();

    std::unordered_map<PostProcessingEffectType, std::shared_ptr<PostProcessingEffect>> m_Effects;
    std::unordered_map<std::string, std::shared_ptr<FlipbookEffect>> m_FlipbookEffects;

    std::shared_ptr<graphics::MeshBuffer> m_FullscreenQuadMeshBuffer;
    std::shared_ptr<graphics::MeshBuffer> m_FlipbookQuadMeshBuffer;
};