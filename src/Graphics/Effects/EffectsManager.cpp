#include "EffectsManager.h"
#include "Scene/Screen.h"
#include "Graphics/Effects/PostProcessingEffects/EdgeDetectionEffect.h"
#include "Graphics/Effects/PostProcessingEffects/NoPostProcessingEffect.h"
#include "Utilities/Logger.h"

EffectsManager& EffectsManager::GetInstance()
{
    static EffectsManager instance;
    return instance;
}

EffectsManager::EffectsManager()
{
    SetupFullscreenQuad();
}

EffectsManager::~EffectsManager()
{
}

std::shared_ptr<PostProcessingEffect> EffectsManager::GetEffect(PostProcessingEffectType effectType)
{
    auto it = m_Effects.find(effectType);
    if (it != m_Effects.end()) {
        return it->second;
    }
    else {
        if (effectType == PostProcessingEffectType::EdgeDetection)
        {
            m_Effects[PostProcessingEffectType::EdgeDetection] = std::make_shared<EdgeDetectionEffect>(m_FullscreenQuadMeshBuffer, Screen::s_Width, Screen::s_Height);
            return m_Effects[PostProcessingEffectType::EdgeDetection];
        }
        else if (effectType == PostProcessingEffectType::None)
        {
            m_Effects[PostProcessingEffectType::None] = std::make_shared<NoPostProcessingEffect>(m_FullscreenQuadMeshBuffer, Screen::s_Width, Screen::s_Height);
            return m_Effects[PostProcessingEffectType::None];
        }
    }

    return nullptr;
}

void EffectsManager::SetEffectParameters(PostProcessingEffectType effectType, const std::unordered_map<std::string, float>& params)
{
    auto effect = GetEffect(effectType);
    if (effect) {
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

void EffectsManager::SetupFullscreenQuad()
{
    auto& resourceManager = ResourceManager::GetInstance();

    auto& meshManager = MeshManager::GetInstance();
    auto quadMesh = meshManager.GetMesh("quad");

    MeshLayout quadMeshLayout = {
        true,  // Positions (vec2)
        false, // Normals
        false, // Tangents
        false, // Bitangents
        { TextureType::Albedo } // Texture Coordinates
    };

    //maybe this should be handled through resource manager too
    m_FullscreenQuadMeshBuffer = std::make_shared<MeshBuffer>(*quadMesh, quadMeshLayout);

    if (!m_FullscreenQuadMeshBuffer) {
        Logger::GetLogger()->error("Failed to set up fullscreen quad mesh buffer.");
    }
    else {
        Logger::GetLogger()->info("Fullscreen quad mesh buffer set up successfully.");
    }
}