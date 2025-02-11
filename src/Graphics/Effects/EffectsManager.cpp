#include "EffectsManager.h"
#include "Scene/Screen.h"
#include "Resources/MeshManager.h"
#include "Resources/ResourceManager.h"
#include "Graphics/Effects/PostProcessingEffects/EdgeDetectionEffect.h"
#include "Graphics/Effects/PostProcessingEffects/NoPostProcessingEffect.h"
#include "Graphics/Effects/PostProcessingEffects/PresentTextureEffect.h"
#include "Graphics/Effects/PostProcessingEffects/ToneMappingEffect.h"
#include "Utilities/Logger.h"
#include <fstream>
#include <stdexcept>
#include <filesystem>

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
        if (effectType == PostProcessingEffectType::EdgeDetection) {
            m_Effects[PostProcessingEffectType::EdgeDetection] = std::make_shared<EdgeDetectionEffect>(m_FullscreenQuadMeshBuffer, Screen::s_Width, Screen::s_Height);
            return m_Effects[PostProcessingEffectType::EdgeDetection];
        }
        else if (effectType == PostProcessingEffectType::PresentTexture) {
            m_Effects[PostProcessingEffectType::PresentTexture] = std::make_shared<PresentTextureEffect>(m_FullscreenQuadMeshBuffer, Screen::s_Width, Screen::s_Height);
            return m_Effects[PostProcessingEffectType::PresentTexture];
        }
        else if (effectType == PostProcessingEffectType::None) {
            m_Effects[PostProcessingEffectType::None] = std::make_shared<NoPostProcessingEffect>(m_FullscreenQuadMeshBuffer, Screen::s_Width, Screen::s_Height);
            return m_Effects[PostProcessingEffectType::None];
        }
        else if (effectType == PostProcessingEffectType::ToneMapping) {
            m_Effects[PostProcessingEffectType::ToneMapping] = std::make_shared<ToneMappingEffect>(m_FullscreenQuadMeshBuffer, Screen::s_Width, Screen::s_Height);
            return m_Effects[PostProcessingEffectType::ToneMapping];
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

    LoadFlipbookEffect(name);
    return m_FlipbookEffects[name];
}

void EffectsManager::LoadFlipbookEffect(const std::string& name) {
    auto cfg = LoadFlipbookConfig(name);

    auto effect = std::make_shared<FlipbookEffect>(m_FlipbookQuadMeshBuffer);
    effect->LoadConfig(cfg.basePath, cfg.framesFile, cfg.totalFrames, cfg.gridX, cfg.gridY, cfg.framesPerSecond, cfg.loop);
    m_FlipbookEffects[name] = effect;
}

namespace fs = std::filesystem;
using json = nlohmann::json;

void LogCurrentWorkingDirectory() {
    auto logger = Logger::GetLogger();
    try {
        fs::path cwd = fs::current_path();
        logger->info("Current Working Directory: '{}'.", cwd.string());
    }
    catch (const std::exception& e) {
        logger->error("Failed to get current working directory: {}", e.what());
    }
}

FlipbookEffectConfig EffectsManager::LoadFlipbookConfig(const std::string& name) {
    auto logger = Logger::GetLogger();

    fs::path basePath = fs::path("..") / "assets" / "VFX" / name;
    fs::path configPath = basePath / (name + ".json");

    logger->info("Loading flipbook config from '{}'.", configPath.string());

    if (!fs::exists(configPath)) {
        logger->error("EffectsManager: Config file '{}' does not exist.", configPath.string());
        throw std::runtime_error("Config file does not exist");
    }

    std::ifstream file(configPath);
    if (!file.is_open()) {
        logger->error("EffectsManager: Could not open config file '{}'.", configPath.string());
        throw std::runtime_error("Cannot open effect config");
    }

    json j;
    file >> j;

    FlipbookEffectConfig cfg;
    cfg.effectType = j.value("effectType", "FlipbookEffect");
    cfg.framesFile = j.value("framesFile", "");
    cfg.totalFrames = j.value("totalFrames", 64);
    cfg.gridX = j.value("gridX", 8); // Correct usage of "gridX"
    cfg.gridY = j.value("gridY", 8); // Correct usage of "gridY"
    cfg.framesPerSecond = j.value("framesPerSecond", 30.0f);
    cfg.loop = j.value("loop", false);
    cfg.basePath = basePath.string() + "/";

    if (cfg.framesFile.empty()) {
        logger->error("EffectsManager: 'framesFile' missing in '{}'.", configPath.string());
        throw std::runtime_error("'framesFile' not found in config");
    }

    logger->info("Flipbook config loaded: effectType={}, framesFile={}, totalFrames={}, gridX={}, gridY={}, fps={}, loop={}",
        cfg.effectType, cfg.framesFile, cfg.totalFrames, cfg.gridX, cfg.gridY, cfg.framesPerSecond, cfg.loop);

    return cfg;
}

void EffectsManager::SetupFullscreenQuad() {
    auto& meshManager = Graphics::MeshManager::GetInstance();
    auto quadMesh = meshManager.GetMesh("quad");
    MeshLayout quadMeshLayout = {
        true,  // Positions
        false, // Normals
        false, // Tangents
        false, // Bitangents
        { TextureType::Albedo }
    };
    m_FullscreenQuadMeshBuffer = std::make_shared<Graphics::MeshBuffer>(*quadMesh, quadMeshLayout);
    m_FlipbookQuadMeshBuffer = m_FullscreenQuadMeshBuffer;
}

void EffectsManager::SetupFlipbookQuad() {
    // Currently using the same quad as fullscreen quad
    // If needed, create a separate quad here.
}