#include "EffectsManager.h"
#include "Graphics/Effects/PostProcessingEffects/EdgeDetectionEffect.h"
// Include other effect headers as needed
#include "Utilities/Logger.h"

EffectsManager& EffectsManager::GetInstance()
{
    static EffectsManager instance;
    return instance;
}

EffectsManager::EffectsManager()
{
    CreateEffects();
}

EffectsManager::~EffectsManager()
{
}

void EffectsManager::CreateEffects()
{
    // Create and store instances of effects
    m_Effects[PostProcessingEffectType::EdgeDetection] = std::make_shared<EdgeDetectionEffect>();
    // Add other effects as needed
}

void EffectsManager::Initialize(int width, int height)
{
    for (auto& [type, effect] : m_Effects) {
        effect->Initialize();
        effect->OnWindowResize(width, height);
    }
}

std::shared_ptr<PostProcessingEffect> EffectsManager::GetEffect(PostProcessingEffectType effectType)
{
    auto it = m_Effects.find(effectType);
    if (it != m_Effects.end()) {
        return it->second;
    }
    return nullptr;
}

void EffectsManager::SetEffectParameters(PostProcessingEffectType effectType, const std::unordered_map<std::string, float>& params)
{
    auto effect = GetEffect(effectType);
    if (effect) {
        // Implement a method in PostProcessingEffect to set parameters
        effect->SetParameters(params);
    }
    else {
        Logger::GetLogger()->warn("Effect type {} not found.", static_cast<int>(effectType));
    }
}

void EffectsManager::OnWindowResize(int width, int height)
{
    for (auto& [type, effect] : m_Effects) {
        effect->OnWindowResize(width, height);
    }
}