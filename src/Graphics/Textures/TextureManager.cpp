#include "TextureManager.h"
#include "TextureUtils.h"
#include "BitMap.h"
#include "TextureLoader.h"
#include "EnvMapPreprocessor.h"
#include "Utilities/Logger.h"
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <nlohmann/json.hpp>
#include <filesystem>

namespace graphics {

    std::string TextureManager::ToLower(const std::string& str) {
        std::string lowerStr = str;
        std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
        return lowerStr;
    }

    bool TextureManager::DetermineSRGB(const std::string&) {
        return false;
    }

    TextureManager& TextureManager::GetInstance() {
        static TextureManager instance;
        return instance;
    }

    TextureManager::TextureManager(const std::filesystem::path& configPath) {
        LoadConfig(configPath);
    }

    bool TextureManager::LoadConfig(const std::filesystem::path& configPath) {
        if (!std::filesystem::exists(configPath)) {
            Logger::GetLogger()->error("TextureManager: Config file '{}' does not exist.", configPath.string());
            return false;
        }
        std::ifstream file(configPath);
        if (!file.good()) {
            Logger::GetLogger()->error("TextureManager: Failed to open config file '{}'.", configPath.string());
            return false;
        }
        nlohmann::json jsonData;
        try {
            file >> jsonData;
        }
        catch (const std::exception& e) {
            Logger::GetLogger()->error("TextureManager: Error parsing config '{}': {}", configPath.string(), e.what());
            return false;
        }
        if (!jsonData.contains("textures")) {
            Logger::GetLogger()->error("TextureManager: JSON config missing 'textures' section.");
            return false;
        }
        auto& texturesJson = jsonData["textures"];
        if (texturesJson.contains("2d")) Load2DTextures(texturesJson["2d"]);
        if (texturesJson.contains("cubeMaps")) LoadCubeMaps(texturesJson["cubeMaps"]);
        if (texturesJson.contains("computed")) LoadComputedTextures(texturesJson["computed"]);
        return true;
    }

    bool TextureManager::Load2DTextures(const nlohmann::json& json) {
        for (auto& [name, value] : json.items()) {
            std::string path;
            if (value.is_string())
                path = value.get<std::string>();
            else if (value.is_object())
                path = value.value("path", "");
            if (path.empty()) {
                Logger::GetLogger()->error("TextureManager: 2D texture '{}' has an invalid path.", name);
                continue;
            }
            TextureConfig config;
            try {
                auto tex = TextureLoader::Load2DTexture(path, config);
                textures_[name] = tex;
                Logger::GetLogger()->info("TextureManager: Loaded 2D texture '{}' from '{}'.", name, path);
            }
            catch (const std::exception& e) {
                Logger::GetLogger()->error("TextureManager: Failed to load 2D texture '{}': {}", name, e.what());
            }
        }
        return true;
    }

    bool TextureManager::LoadCubeMaps(const nlohmann::json& json) {
        for (auto& [cubeMapName, value] : json.items()) {
            if (!value.is_array() || value.empty()) {
                Logger::GetLogger()->error("TextureManager: Cubemap '{}' should be a non-empty array.", cubeMapName);
                continue;
            }
            if (value.size() == 1) {
                std::string eqPath = value[0].get<std::string>();
                if (!ConvertAndLoadEquirectHDR(cubeMapName, eqPath))
                    Logger::GetLogger()->error("TextureManager: Failed to convert equirect HDR for '{}'.", cubeMapName);
            }
            else if (value.size() == 6) {
                std::array<std::string, 6> facePaths;
                for (size_t i = 0; i < 6; ++i) {
                    facePaths[i] = value[i].get<std::string>();
                }
                TextureConfig cfg;
                try {
                    auto tex = TextureLoader::LoadCubeMapTexture(facePaths, cfg);
                    textures_[cubeMapName] = tex;
                    Logger::GetLogger()->info("TextureManager: Loaded cubemap '{}' with 6 faces.", cubeMapName);
                }
                catch (const std::exception& e) {
                    Logger::GetLogger()->error("TextureManager: Failed to load cubemap '{}': {}", cubeMapName, e.what());
                }
            }
            else {
                Logger::GetLogger()->error("TextureManager: Cubemap '{}' must have 1 or 6 paths (got {}).", cubeMapName, value.size());
            }
        }
        return true;
    }

    bool TextureManager::LoadComputedTextures(const nlohmann::json& json) {
        for (auto& [name, info] : json.items()) {
            std::string type = info.value("type", "");
            if (type == "compute" || type == "brdf") {
                int w = info.value("width", 256);
                int h = info.value("height", 256);
                unsigned int samples = info.value("numSamples", 1024);
                try {
                    auto tex = TextureLoader::CreateBRDFTexture(w, h, samples);
                    textures_[name] = tex;
                    Logger::GetLogger()->info("TextureManager: Created computed BRDF texture '{}'.", name);
                }
                catch (const std::exception& e) {
                    Logger::GetLogger()->error("TextureManager: Exception creating computed texture '{}': {}", name, e.what());
                }
            }
            else {
                Logger::GetLogger()->error("TextureManager: Unknown computed texture type '{}' for '{}'.", type, name);
            }
        }
        return true;
    }

    std::shared_ptr<ITexture> TextureManager::GetTexture(const std::string& name) {
        auto it = textures_.find(name);
        if (it != textures_.end()) return it->second;
        Logger::GetLogger()->warn("TextureManager: Texture '{}' not found.", name);
        return nullptr;
    }

    std::shared_ptr<ITexture> TextureManager::Load2DTexture(const std::string& name, const std::string& path, const TextureConfig& config) {
        if (auto existing = GetTexture(name)) return existing;
        try {
            auto tex = TextureLoader::Load2DTexture(path, config);
            textures_[name] = tex;
            return tex;
        }
        catch (const std::exception& e) {
            Logger::GetLogger()->error("TextureManager: Exception loading 2D texture '{}': {}", name, e.what());
            return nullptr;
        }
    }

    std::shared_ptr<ITexture> TextureManager::LoadCubeMapTexture(const std::string& name, const std::array<std::string, 6>& facePaths, const TextureConfig& config) {
        if (auto existing = GetTexture(name)) return existing;
        try {
            auto tex = TextureLoader::LoadCubeMapTexture(facePaths, config);
            textures_[name] = tex;
            return tex;
        }
        catch (const std::exception& e) {
            Logger::GetLogger()->error("TextureManager: Exception loading cubemap '{}': {}", name, e.what());
            return nullptr;
        }
    }

    std::shared_ptr<ITexture> TextureManager::LoadTextureArray(const std::string& name, const std::string& path, const TextureConfig& config, uint32_t totalFrames, uint32_t gridX, uint32_t gridY) {
        if (auto existing = GetTexture(name)) return existing;
        try {
            auto tex = TextureLoader::LoadTextureArray(path, config, totalFrames, gridX, gridY);
            textures_[name] = tex;
            return tex;
        }
        catch (const std::exception& e) {
            Logger::GetLogger()->error("TextureManager: Exception loading texture array '{}': {}", name, e.what());
            return nullptr;
        }
    }

    std::shared_ptr<ITexture> TextureManager::CreateBRDFTexture(const std::string& name, int width, int height, unsigned int numSamples) {
        if (auto existing = GetTexture(name)) return existing;
        try {
            auto tex = TextureLoader::CreateBRDFTexture(width, height, numSamples);
            textures_[name] = tex;
            return tex;
        }
        catch (const std::exception& e) {
            Logger::GetLogger()->error("TextureManager: Exception creating BRDF texture '{}': {}", name, e.what());
            return nullptr;
        }
    }

    bool TextureManager::ConvertAndLoadEquirectHDR(const std::string& cubeMapName, const std::string& equirectPath) {
        std::filesystem::path srcPath(equirectPath);
        if (!std::filesystem::exists(srcPath)) {
            Logger::GetLogger()->error("TextureManager: Equirect HDR '{}' does not exist.", equirectPath);
            return false;
        }
        std::string stem = srcPath.stem().string();
        std::filesystem::path outDir = srcPath.parent_path() / stem;
        std::array<std::filesystem::path, 6> envFaces = { outDir / "face_0.hdr", outDir / "face_1.hdr", outDir / "face_2.hdr",
                                                         outDir / "face_3.hdr", outDir / "face_4.hdr", outDir / "face_5.hdr" };
        std::array<std::filesystem::path, 6> ldrFaces = { outDir / "ldr_face_0.png", outDir / "ldr_face_1.png", outDir / "ldr_face_2.png",
                                                         outDir / "ldr_face_3.png", outDir / "ldr_face_4.png", outDir / "ldr_face_5.png" };
        std::array<std::filesystem::path, 6> irrFaces = { outDir / "irr_0.hdr", outDir / "irr_1.hdr", outDir / "irr_2.hdr",
                                                         outDir / "irr_3.hdr", outDir / "irr_4.hdr", outDir / "irr_5.hdr" };
        if (!std::filesystem::exists(outDir)) {
            std::error_code ec;
            std::filesystem::create_directories(outDir, ec);
            if (ec) {
                Logger::GetLogger()->error("TextureManager: Cannot create directory '{}': {}", outDir.string(), ec.message());
                return false;
            }
        }
        EnvMapPreprocessor preprocessor;
        if (!std::filesystem::exists(envFaces[0])) {
            Bitmap equirect = preprocessor.LoadTexture(srcPath);
            Bitmap vCross = preprocessor.ConvertEquirectangularMapToVerticalCross(equirect);
            Bitmap faceCube = preprocessor.ConvertVerticalCrossToCubeMapFaces(vCross);
            SaveFacesToDisk(faceCube, envFaces, "face_");
            Logger::GetLogger()->info("TextureManager: Created environment (HDR) faces for '{}'.", equirectPath);
        }
        else {
            Logger::GetLogger()->info("TextureManager: Environment faces already exist for '{}'.", equirectPath);
        }
        if (!std::filesystem::exists(ldrFaces[0])) {
            Logger::GetLogger()->info("TextureManager: Generating LDR skybox faces for '{}'.", equirectPath);
            Bitmap equirect = preprocessor.LoadTexture(srcPath);
            Bitmap vCross = preprocessor.ConvertEquirectangularMapToVerticalCross(equirect);
            Bitmap faceCube = preprocessor.ConvertVerticalCrossToCubeMapFaces(vCross);
            SaveFacesToDisk(faceCube, ldrFaces, "ldr_face_");
            Logger::GetLogger()->info("TextureManager: Created LDR skybox faces for '{}'.", equirectPath);
        }
        else {
            Logger::GetLogger()->info("TextureManager: LDR skybox faces already exist for '{}'.", equirectPath);
        }
        if (!std::filesystem::exists(irrFaces[0])) {
            Logger::GetLogger()->info("TextureManager: Generating irradiance faces for '{}'.", equirectPath);
            Bitmap equirect = preprocessor.LoadTexture(srcPath);
            Bitmap equirectIrr = preprocessor.ComputeIrradianceEquirect(equirect, 256, 128, 1024);
            Bitmap vCrossIrr = preprocessor.ConvertEquirectangularMapToVerticalCross(equirectIrr);
            Bitmap faceCubeIrr = preprocessor.ConvertVerticalCrossToCubeMapFaces(vCrossIrr);
            SaveFacesToDisk(faceCubeIrr, irrFaces, "irr_");
            Logger::GetLogger()->info("TextureManager: Created irradiance faces for '{}'.", equirectPath);
        }
        else {
            Logger::GetLogger()->info("TextureManager: Irradiance faces already exist for '{}'.", equirectPath);
        }
        TextureConfig envCfg = MakeSomeCubeMapConfig(true);
        try {
            auto cubeMap = TextureLoader::LoadCubeMapTexture({ envFaces[0].string(), envFaces[1].string(), envFaces[2].string(),
                                                               envFaces[3].string(), envFaces[4].string(), envFaces[5].string() }, envCfg);
            textures_[cubeMapName] = cubeMap;
        }
        catch (const std::exception& e) {
            Logger::GetLogger()->error("TextureManager: Exception creating env cubemap '{}': {}", cubeMapName, e.what());
            return false;
        }
        TextureConfig irrCfg = MakeSomeCubeMapConfig(true);
        try {
            auto irrCube = TextureLoader::LoadCubeMapTexture({ irrFaces[0].string(), irrFaces[1].string(), irrFaces[2].string(),
                                                               irrFaces[3].string(), irrFaces[4].string(), irrFaces[5].string() }, irrCfg);
            textures_[cubeMapName + "_irr"] = irrCube;
            Logger::GetLogger()->info("TextureManager: Created irradiance cubemap '{}_irr'.", cubeMapName);
        }
        catch (const std::exception& e) {
            Logger::GetLogger()->warn("TextureManager: Failed to create irradiance cubemap for '{}': {}", cubeMapName, e.what());
        }
        TextureConfig skyboxCfg;
        skyboxCfg.internal_format_ = GL_SRGB8_ALPHA8;
        skyboxCfg.wrap_s_ = GL_CLAMP_TO_EDGE;
        skyboxCfg.wrap_t_ = GL_CLAMP_TO_EDGE;
        skyboxCfg.wrap_r_ = GL_CLAMP_TO_EDGE;
        skyboxCfg.min_filter_ = GL_LINEAR_MIPMAP_LINEAR;
        skyboxCfg.mag_filter_ = GL_LINEAR;
        skyboxCfg.generate_mips_ = true;
        skyboxCfg.use_anisotropy_ = true;
        skyboxCfg.use_bindless_ = false;
        skyboxCfg.is_hdr_ = false;
        skyboxCfg.is_srgb_ = true;
        try {
            auto skyboxCube = TextureLoader::LoadCubeMapTexture({ ldrFaces[0].string(), ldrFaces[1].string(), ldrFaces[2].string(),
                                                                 ldrFaces[3].string(), ldrFaces[4].string(), ldrFaces[5].string() }, skyboxCfg);
            textures_[cubeMapName + "_skybox"] = skyboxCube;
            Logger::GetLogger()->info("TextureManager: Loaded skybox cubemap '{}_skybox'.", cubeMapName);
        }
        catch (const std::exception& e) {
            Logger::GetLogger()->error("TextureManager: Exception creating skybox cubemap '{}': {}", cubeMapName, e.what());
        }
        return true;
    }

    TextureConfig TextureManager::MakeSomeCubeMapConfig(bool isHDR) {
        TextureConfig cfg;
        cfg.internal_format_ = GL_RGB32F;
        cfg.wrap_s_ = GL_CLAMP_TO_EDGE;
        cfg.wrap_t_ = GL_CLAMP_TO_EDGE;
        cfg.wrap_r_ = GL_CLAMP_TO_EDGE;
        cfg.min_filter_ = GL_LINEAR_MIPMAP_LINEAR;
        cfg.mag_filter_ = GL_LINEAR;
        cfg.generate_mips_ = true;
        cfg.use_anisotropy_ = true;
        cfg.use_bindless_ = false;
        cfg.is_hdr_ = isHDR;
        cfg.is_srgb_ = false;
        return cfg;
    }

    void TextureManager::SaveFacesToDisk(const Bitmap& cubeMap, const std::array<std::filesystem::path, 6>& facePaths, const std::string& prefix) {
        if (cubeMap.depth() != 6) {
            Logger::GetLogger()->error("TextureManager: SaveFacesToDisk: expected depth=6, got {}", cubeMap.depth());
            return;
        }
        int faceW = cubeMap.width(), faceH = cubeMap.height(), comp = cubeMap.components();
        int pixBytes = comp * Bitmap::getBytesPerComponent(cubeMap.format());
        int faceSize = faceW * faceH * pixBytes;
        EnvMapPreprocessor preprocessor;
        for (int i = 0; i < 6; ++i) {
            Bitmap faceBmp(faceW, faceH, comp, cubeMap.format());
            const uint8_t* src = cubeMap.data().data() + i * faceSize;
            std::memcpy(faceBmp.data().data(), src, faceSize);
            try {
                SaveAsHDR(faceBmp, facePaths[i]);
            }
            catch (const std::exception& e) {
                Logger::GetLogger()->error("TextureManager: SaveFacesToDisk: face {}: {}", prefix + std::to_string(i), e.what());
            }
        }
    }

    void TextureManager::Clear() {
        textures_.clear();
        Logger::GetLogger()->info("TextureManager: Cleared texture cache.");
    }

} // namespace graphics