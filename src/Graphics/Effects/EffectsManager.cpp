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
namespace fs = std::filesystem;

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
    for (auto& [type, effect] : effects_) {
        effect->OnWindowResize(width, height);
    }
}

std::shared_ptr<PostProcessingEffect> EffectsManager::GetEffect(PostProcessingEffectType effectType) {
    auto it = effects_.find(effectType);
    if (it != effects_.end()) {
        return it->second;
    }
    else {
        if (effectType == PostProcessingEffectType::EdgeDetection) {
            effects_[PostProcessingEffectType::EdgeDetection] = std::make_shared<EdgeDetectionEffect>(fullscreenQuadMeshBuffer_, Screen::width_, Screen::height_);
            return effects_[PostProcessingEffectType::EdgeDetection];
        }
        else if (effectType == PostProcessingEffectType::PresentTexture) {
            effects_[PostProcessingEffectType::PresentTexture] = std::make_shared<PresentTextureEffect>(fullscreenQuadMeshBuffer_, Screen::width_, Screen::height_);
            return effects_[PostProcessingEffectType::PresentTexture];
        }
        else if (effectType == PostProcessingEffectType::None) {
            effects_[PostProcessingEffectType::None] = std::make_shared<NoPostProcessingEffect>(fullscreenQuadMeshBuffer_, Screen::width_, Screen::height_);
            return effects_[PostProcessingEffectType::None];
        }
        else if (effectType == PostProcessingEffectType::ToneMapping) {
            effects_[PostProcessingEffectType::ToneMapping] = std::make_shared<ToneMappingEffect>(fullscreenQuadMeshBuffer_, Screen::width_, Screen::height_);
            return effects_[PostProcessingEffectType::ToneMapping];
        }
    }
    return nullptr;
}

void EffectsManager::SetEffectParameters(PostProcessingEffectType effectType, const std::unordered_map<std::string, EffectParameter>& params) {
    auto effect = GetEffect(effectType);
    if (effect) {
        effect->SetParameters(params);
    }
    else {
        Logger::GetLogger()->warn("Effect type {} not found.", static_cast<int>(effectType));
    }
}

std::shared_ptr<FlipbookEffect> EffectsManager::GetFlipbookEffect(const std::string& name) {
    auto it = flipbookEffects_.find(name);
    if (it != flipbookEffects_.end()) {
        return it->second;
    }
    LoadFlipbookEffect(name);
    return flipbookEffects_[name];
}

void EffectsManager::LoadFlipbookEffect(const std::string& name) {
    auto cfg = LoadFlipbookConfig(name);
    auto effect = std::make_shared<FlipbookEffect>(flipbookQuadMeshBuffer_);
    effect->LoadConfig(cfg.basePath, cfg.framesFile, cfg.totalFrames, cfg.gridX, cfg.gridY, cfg.framesPerSecond, cfg.loop);
    flipbookEffects_[name] = effect;
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
    cfg.gridX = j.value("gridX", 8);
    cfg.gridY = j.value("gridY", 8);
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
    auto& meshManager = graphics::MeshManager::GetInstance();
    auto quadMesh = meshManager.GetMesh("quad");
    MeshLayout quadMeshLayout = {
        true,  // Positions
        false, // Normals
        false, // Tangents
        false, // Bitangents
        { TextureType::Diffuse }
    };
    fullscreenQuadMeshBuffer_ = std::make_shared<graphics::MeshBuffer>(*quadMesh, quadMeshLayout);
    flipbookQuadMeshBuffer_ = fullscreenQuadMeshBuffer_;
}

void EffectsManager::SetupFlipbookQuad() {
    // Using same quad as fullscreen quad; adjust if separate quad is needed.
}