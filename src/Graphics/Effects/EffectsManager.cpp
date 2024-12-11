#include "EffectsManager.h"
#include "Scene/Screen.h"
#include "Resources/MeshManager.h"
#include "Resources/ResourceManager.h"
#include "Graphics/Effects/PostProcessingEffects/EdgeDetectionEffect.h"
#include "Graphics/Effects/PostProcessingEffects/NoPostProcessingEffect.h"
#include "Graphics/Effects/PostProcessingEffects/PresentTextureEffect.h"
#include "Utilities/Logger.h"
#include <fstream>
#include <stdexcept>

// Use nlohmann::json for parsing
#include <nlohmann/json.hpp>
using json = nlohmann::json;

EffectsManager& EffectsManager::GetInstance() {
    static EffectsManager instance;
    return instance;
}

EffectsManager::EffectsManager() {
    SetupFullscreenQuad();
    SetupFlipbookQuad();
}

EffectsManager::~EffectsManager() {}

void EffectsManager::OnWindowResize(int width, int height) {
    for (auto& [type, effect] : m_Effects) {
        effect->OnWindowResize(width, height);
    }
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
        else if (effectType == PostProcessingEffectType::PresentTexture)
        {
            m_Effects[PostProcessingEffectType::PresentTexture] = std::make_shared<PresentTextureEffect>(m_FullscreenQuadMeshBuffer, Screen::s_Width, Screen::s_Height);
            return m_Effects[PostProcessingEffectType::PresentTexture];
        }
        else if (effectType == PostProcessingEffectType::None)
        {
            m_Effects[PostProcessingEffectType::None] = std::make_shared<NoPostProcessingEffect>(m_FullscreenQuadMeshBuffer, Screen::s_Width, Screen::s_Height);
            return m_Effects[PostProcessingEffectType::None];
        }
    }

    return nullptr;
}

void EffectsManager::SetEffectParameters(PostProcessingEffectType effectType, const std::unordered_map<std::string, EffectParameter>& params)
{
    auto effect = GetEffect(effectType);
    if (effect) {
        effect->SetParameters(params);
    }
    else {
        Logger::GetLogger()->warn("Effect type {} not found.", static_cast<int>(effectType));
    }
}

std::shared_ptr<FlipbookEffect> EffectsManager::GetFlipbookEffect(const std::string& name) {
    auto it = m_FlipbookEffects.find(name);
    if (it != m_FlipbookEffects.end()) {
        return it->second;
    }

    // Not found, try loading
    LoadFlipbookEffect(name);
    return m_FlipbookEffects[name];
}

void EffectsManager::LoadFlipbookEffect(const std::string& name) {
    // Load config
    auto cfg = LoadFlipbookConfig(name);

    auto effect = std::make_shared<FlipbookEffect>(m_FlipbookQuadMeshBuffer);
    effect->LoadConfig(cfg.basePath, cfg.framesFile, cfg.totalFrames, cfg.framesPerSecond, cfg.loop);
    m_FlipbookEffects[name] = effect;
}

FlipbookEffectConfig EffectsManager::LoadFlipbookConfig(const std::string& name) {
    // Construct path to JSON config
    // E.g. "assets/VFX/FireBall04-flipbooks/FireBall04.json"
    std::string basePath = "assets/VFX/" + name + "-flipbooks/";
    std::string configPath = basePath + name + ".json";

    std::ifstream file(configPath);
    if (!file.is_open()) {
        Logger::GetLogger()->error("EffectsManager: Could not open config file {}", configPath);
        throw std::runtime_error("Cannot open effect config");
    }

    json j;
    file >> j;

    FlipbookEffectConfig cfg;
    cfg.effectType = j.value("effectType", "FlipbookEffect");
    cfg.framesFile = j.value("framesFile", "");
    cfg.totalFrames = j.value("totalFrames", 64);
    cfg.framesPerSecond = j.value("framesPerSecond", 30.0f);
    cfg.loop = j.value("loop", false);
    cfg.basePath = basePath;

    if (cfg.framesFile.empty()) {
        Logger::GetLogger()->error("EffectsManager: framesFile missing in {}", configPath);
        throw std::runtime_error("framesFile not found in config");
    }

    return cfg;
}

void EffectsManager::SetupFullscreenQuad() {
    auto& meshManager = MeshManager::GetInstance();
    auto quadMesh = meshManager.GetMesh("quad");
    MeshLayout quadMeshLayout = {
        true,  // Positions (vec2)
        false, // Normals
        false, // Tangents
        false, // Bitangents
        { TextureType::Albedo }
    };
    m_FullscreenQuadMeshBuffer = std::make_shared<MeshBuffer>(*quadMesh, quadMeshLayout);
    m_FlipbookQuadMeshBuffer = m_FullscreenQuadMeshBuffer;
}

void EffectsManager::SetupFlipbookQuad()
{
}