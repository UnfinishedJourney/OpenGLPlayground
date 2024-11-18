#pragma once

#include <memory>
#include <unordered_map>
#include "Graphics/Effects/PostProcessingEffects/PostProcessingEffect.h"
//#include "PostProcessingEffectType.h"

class EffectsManager {
public:
    static EffectsManager& GetInstance();

    void Initialize(int width, int height);

    std::shared_ptr<PostProcessingEffect> GetEffect(PostProcessingEffectType effectType);

    void SetEffectParameters(PostProcessingEffectType effectType, const std::unordered_map<std::string, float>& params);

    void OnWindowResize(int width, int height);

private:
    EffectsManager();
    ~EffectsManager();

    // Prevent copying
    EffectsManager(const EffectsManager&) = delete;
    EffectsManager& operator=(const EffectsManager&) = delete;

    void CreateEffects();

    std::unordered_map<PostProcessingEffectType, std::shared_ptr<PostProcessingEffect>> m_Effects;
};