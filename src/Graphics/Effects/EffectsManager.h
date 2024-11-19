#pragma once

#include <memory>
#include <unordered_map>
#include "Graphics/Effects/PostProcessingEffects/PostProcessingEffect.h"
#include "Graphics/Effects/PostProcessingEffects/PostProcessingEffectType.h"

class EffectsManager {
public:
    static EffectsManager& GetInstance();

    std::shared_ptr<PostProcessingEffect> GetEffect(PostProcessingEffectType effectType);

    void SetEffectParameters(PostProcessingEffectType effectType, const std::unordered_map<std::string, float>& params);

    void OnWindowResize(int width, int height);

private:
    EffectsManager();
    ~EffectsManager();

    EffectsManager(const EffectsManager&) = delete;
    EffectsManager& operator=(const EffectsManager&) = delete;

    std::unordered_map<PostProcessingEffectType, std::shared_ptr<PostProcessingEffect>> m_Effects;
    std::shared_ptr<MeshBuffer> m_FullscreenQuadMeshBuffer;
    void SetupFullscreenQuad();
};