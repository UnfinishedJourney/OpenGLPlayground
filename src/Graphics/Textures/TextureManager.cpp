#include "TextureManager.h"
#include "OpenGLTexture.h"
#include "OpenGLCubeMapTexture.h"
#include "EnvMapPreprocessor.h"
#include "Graphics/Shaders/ShaderManager.h"
#include "Graphics/Shaders/ComputeShader.h"
#include "Graphics/Buffers/ShaderStorageBuffer.h"
#include "TextureData.h"
#include "Bitmap.h"
#include "Utilities/Logger.h"
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <nlohmann/json.hpp>
#include <stb_image.h>
#include <filesystem>

namespace Graphics {

    std::string TextureManager::ToLower(const std::string& str) {
        std::string lowerStr = str;
        std::transform(str.begin(), str.end(), lowerStr.begin(), ::tolower);
        return lowerStr;
    }

    bool TextureManager::IsHDRTexture(const std::filesystem::path& path) {
        static const std::vector<std::string> hdrExts = { ".hdr", ".exr", ".tif", ".tiff" };
        std::string ext = ToLower(path.extension().string());
        return std::find(hdrExts.begin(), hdrExts.end(), ext) != hdrExts.end();
    }

    bool TextureManager::DetermineSRGB(const std::string& /*pathStr*/) {
        // Placeholder – adjust based on naming conventions if desired.
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
            Logger::GetLogger()->error("Texture config file '{}' does not exist.", configPath.string());
            return false;
        }
        std::ifstream file(configPath);
        if (!file.good()) {
            Logger::GetLogger()->error("Failed to open texture config file '{}'.", configPath.string());
            return false;
        }
        nlohmann::json jsonData;
        try {
            file >> jsonData;
        }
        catch (const std::exception& e) {
            Logger::GetLogger()->error("Error parsing texture config '{}': {}", configPath.string(), e.what());
            return false;
        }
        if (!jsonData.contains("textures")) {
            Logger::GetLogger()->error("JSON config missing 'textures' object.");
            return false;
        }
        auto& texturesJson = jsonData["textures"];
        if (texturesJson.contains("2d")) {
            if (!Load2DTextures(texturesJson["2d"])) return false;
        }
        if (texturesJson.contains("cubeMaps")) {
            if (!LoadCubeMaps(texturesJson["cubeMaps"])) return false;
        }
        if (texturesJson.contains("arrays")) {
            if (!LoadTextureArrays(texturesJson["arrays"])) return false;
        }
        if (texturesJson.contains("computed")) {
            if (!LoadComputedTextures(texturesJson["computed"])) return false;
        }
        return true;
    }

    bool TextureManager::Load2DTextures(const nlohmann::json& json) {
        for (auto& [name, value] : json.items()) {
            std::string path;
            if (value.is_string()) {
                path = value.get<std::string>();
            }
            else if (value.is_object()) {
                path = value.value("path", "");
            }
            if (path.empty()) {
                Logger::GetLogger()->error("2D Texture '{}' has an invalid path.", name);
                continue;
            }
            auto tex = LoadTexture(name, path);
            if (!tex) {
                Logger::GetLogger()->error("Failed loading 2D texture '{}'.", name);
            }
            else {
                Logger::GetLogger()->info("Loaded 2D texture '{}'.", name);
            }
        }
        return true;
    }

    bool TextureManager::LoadCubeMaps(const nlohmann::json& json) {
        for (auto& [cubeMapName, value] : json.items()) {
            if (!value.is_array()) {
                Logger::GetLogger()->error("Cubemap '{}' should be an array of paths.", cubeMapName);
                continue;
            }
            std::vector<std::string> facePaths = value.get<std::vector<std::string>>();
            if (facePaths.empty()) {
                Logger::GetLogger()->error("Cubemap '{}' has an empty path array.", cubeMapName);
                continue;
            }
            if (facePaths.size() == 1) {
                if (!ConvertAndLoadEquirectHDR(cubeMapName, facePaths[0])) {
                    Logger::GetLogger()->error("Failed to convert equirect HDR for '{}'.", cubeMapName);
                }
            }
            else if (facePaths.size() == 6) {
                std::array<std::filesystem::path, 6> faces;
                for (size_t i = 0; i < 6; ++i)
                    faces[i] = facePaths[i];
                TextureConfig cubeMapConfig;
                cubeMapConfig.internalFormat = GL_RGB32F;
                cubeMapConfig.wrapS = GL_CLAMP_TO_EDGE;
                cubeMapConfig.wrapT = GL_CLAMP_TO_EDGE;
                cubeMapConfig.wrapR = GL_CLAMP_TO_EDGE;
                cubeMapConfig.minFilter = GL_LINEAR_MIPMAP_LINEAR;
                cubeMapConfig.magFilter = GL_LINEAR;
                cubeMapConfig.generateMips = true;
                cubeMapConfig.useAnisotropy = true;
                cubeMapConfig.useBindless = false;
                bool anyHDR = false, anySRGB = false;
                for (const auto& face : faces) {
                    anyHDR = anyHDR || IsHDRTexture(face);
                    anySRGB = anySRGB || DetermineSRGB(face.string());
                }
                cubeMapConfig.isHDR = anyHDR;
                cubeMapConfig.isSRGB = anySRGB;
                try {
                    auto cubeMap = std::make_shared<OpenGLCubeMapTexture>(faces, cubeMapConfig);
                    textures_[cubeMapName] = cubeMap;
                    Logger::GetLogger()->info("Loaded 6-face cube map '{}'.", cubeMapName);
                }
                catch (const std::exception& e) {
                    Logger::GetLogger()->error("Exception loading cube map '{}': {}", cubeMapName, e.what());
                }
            }
            else {
                Logger::GetLogger()->error("Cubemap '{}' must have 1 (equirect) or 6 paths. Got {}.", cubeMapName, facePaths.size());
            }
        }
        return true;
    }

    bool TextureManager::LoadTextureArrays(const nlohmann::json& json) {
        Logger::GetLogger()->info("LoadTextureArrays is not implemented.");
        return true;
    }

    bool TextureManager::LoadComputedTextures(const nlohmann::json& json) {
        for (auto& [name, info] : json.items()) {
            std::string type = info.value("type", "");
            if (type == "compute") {
                int width = info.value("width", 256);
                int height = info.value("height", 256);
                unsigned int samples = info.value("numSamples", 1024);
                auto tex = CreateBRDFLUT(width, height, samples);
                if (tex) {
                    textures_[name] = tex;
                    Logger::GetLogger()->info("Computed texture '{}' created (BRDF LUT).", name);
                }
            }
            else {
                Logger::GetLogger()->error("Unknown computed texture type '{}' for '{}'.", type, name);
            }
        }
        return true;
    }

    bool TextureManager::ConvertAndLoadEquirectHDR(const std::string& cubeMapName, const std::string& equirectPath) {
        std::filesystem::path srcPath(equirectPath);
        if (!std::filesystem::exists(srcPath)) {
            Logger::GetLogger()->error("Equirect HDR '{}' does not exist.", equirectPath);
            return false;
        }
        std::string stem = srcPath.stem().string();
        std::filesystem::path outDir = srcPath.parent_path() / stem;
        std::array<std::filesystem::path, 6> envFaces = {
            outDir / "face_0.hdr",
            outDir / "face_1.hdr",
            outDir / "face_2.hdr",
            outDir / "face_3.hdr",
            outDir / "face_4.hdr",
            outDir / "face_5.hdr"
        };
        std::array<std::filesystem::path, 6> ldrFaces = {
            outDir / "ldr_face_0.png",
            outDir / "ldr_face_1.png",
            outDir / "ldr_face_2.png",
            outDir / "ldr_face_3.png",
            outDir / "ldr_face_4.png",
            outDir / "ldr_face_5.png"
        };
        std::array<std::filesystem::path, 6> irrFaces = {
            outDir / "irr_0.hdr",
            outDir / "irr_1.hdr",
            outDir / "irr_2.hdr",
            outDir / "irr_3.hdr",
            outDir / "irr_4.hdr",
            outDir / "irr_5.hdr"
        };

        if (!std::filesystem::exists(outDir)) {
            std::error_code ec;
            std::filesystem::create_directories(outDir, ec);
            if (ec) {
                Logger::GetLogger()->error("Cannot create directory '{}': {}", outDir.string(), ec.message());
                return false;
            }
        }

        EnvMapPreprocessor preprocessor;
        if (!std::filesystem::exists(envFaces[0])) {
            Bitmap equirect = preprocessor.LoadTexture(srcPath);
            Bitmap vCross = preprocessor.ConvertEquirectangularMapToVerticalCross(equirect);
            Bitmap faceCube = preprocessor.ConvertVerticalCrossToCubeMapFaces(vCross);
            SaveFacesToDisk(faceCube, envFaces, "face_");
            Logger::GetLogger()->info("Created environment (HDR) faces for '{}'.", equirectPath);
        }
        else {
            Logger::GetLogger()->info("Environment faces already exist for '{}'.", equirectPath);
        }
        if (!std::filesystem::exists(ldrFaces[0])) {
            Logger::GetLogger()->info("Generating LDR skybox faces for '{}'.", equirectPath);
            Bitmap equirect = preprocessor.LoadTexture(srcPath);
            Bitmap vCross = preprocessor.ConvertEquirectangularMapToVerticalCross(equirect);
            Bitmap faceCube = preprocessor.ConvertVerticalCrossToCubeMapFaces(vCross);
            preprocessor.SaveFacesToDiskLDR(faceCube, ldrFaces, "ldr_face_");
            Logger::GetLogger()->info("Created LDR skybox faces for '{}'.", equirectPath);
        }
        else {
            Logger::GetLogger()->info("LDR skybox faces already exist for '{}'.", equirectPath);
        }
        if (!std::filesystem::exists(irrFaces[0])) {
            Logger::GetLogger()->info("Generating irradiance faces for '{}'.", equirectPath);
            Bitmap equirect = preprocessor.LoadTexture(srcPath);
            Bitmap equirectIrr = preprocessor.ComputeIrradianceEquirect(equirect, 256, 128, 1024);
            Bitmap vCrossIrr = preprocessor.ConvertEquirectangularMapToVerticalCross(equirectIrr);
            Bitmap faceCubeIrr = preprocessor.ConvertVerticalCrossToCubeMapFaces(vCrossIrr);
            SaveFacesToDisk(faceCubeIrr, irrFaces, "irr_");
            Logger::GetLogger()->info("Created irradiance faces for '{}'.", equirectPath);
        }
        else {
            Logger::GetLogger()->info("Irradiance faces already exist for '{}'.", equirectPath);
        }
        TextureConfig envCfg = MakeSomeCubeMapConfig(true);
        try {
            auto cubeMap = std::make_shared<OpenGLCubeMapTexture>(envFaces, envCfg);
            textures_[cubeMapName] = cubeMap;
        }
        catch (const std::exception& e) {
            Logger::GetLogger()->error("Exception creating env cubemap '{}': {}", cubeMapName, e.what());
            return false;
        }
        TextureConfig irrCfg = MakeSomeCubeMapConfig(true);
        try {
            auto irrCube = std::make_shared<OpenGLCubeMapTexture>(irrFaces, irrCfg);
            textures_[cubeMapName + "_irr"] = irrCube;
            Logger::GetLogger()->info("Created irradiance cubemap '{}_irr'.", cubeMapName);
        }
        catch (const std::exception& e) {
            Logger::GetLogger()->warn("Failed to create irradiance cubemap for '{}': {}", cubeMapName, e.what());
        }
        TextureConfig skyboxCfg;
        skyboxCfg.internalFormat = GL_SRGB8_ALPHA8;
        skyboxCfg.wrapS = GL_CLAMP_TO_EDGE;
        skyboxCfg.wrapT = GL_CLAMP_TO_EDGE;
        skyboxCfg.wrapR = GL_CLAMP_TO_EDGE;
        skyboxCfg.minFilter = GL_LINEAR_MIPMAP_LINEAR;
        skyboxCfg.magFilter = GL_LINEAR;
        skyboxCfg.generateMips = true;
        skyboxCfg.useAnisotropy = true;
        skyboxCfg.useBindless = false;
        skyboxCfg.isHDR = false;
        skyboxCfg.isSRGB = true;
        try {
            auto skyboxCube = std::make_shared<OpenGLCubeMapTexture>(ldrFaces, skyboxCfg);
            textures_[cubeMapName + "_skybox"] = skyboxCube;
            Logger::GetLogger()->info("Loaded skybox (LDR) cubemap '{}_skybox'.", cubeMapName);
        }
        catch (const std::exception& e) {
            Logger::GetLogger()->error("Exception creating skybox cubemap '{}': {}", cubeMapName, e.what());
        }
        return true;
    }

    TextureConfig TextureManager::MakeSomeCubeMapConfig(bool isHDR) {
        TextureConfig cfg;
        cfg.internalFormat = GL_RGB32F;
        cfg.wrapS = GL_CLAMP_TO_EDGE;
        cfg.wrapT = GL_CLAMP_TO_EDGE;
        cfg.wrapR = GL_CLAMP_TO_EDGE;
        cfg.minFilter = GL_LINEAR_MIPMAP_LINEAR;
        cfg.magFilter = GL_LINEAR;
        cfg.generateMips = true;
        cfg.useAnisotropy = true;
        cfg.isHDR = isHDR;
        cfg.isSRGB = false;
        return cfg;
    }

    void TextureManager::SaveFacesToDisk(const Bitmap& cubeMap,
        const std::array<std::filesystem::path, 6>& facePaths,
        const std::string& prefix) {
        if (cubeMap.depth() != 6) {
            Logger::GetLogger()->error("SaveFacesToDisk: expected 6 faces, found depth = {}", cubeMap.depth());
            return;
        }
        int faceW = cubeMap.width();
        int faceH = cubeMap.height();
        int comp = cubeMap.components();
        int pixelSize = comp * Bitmap::getBytesPerComponent(cubeMap.format());
        int faceSizeBytes = faceW * faceH * pixelSize;
        EnvMapPreprocessor preprocessor;
        for (int i = 0; i < 6; ++i) {
            Bitmap faceBmp(faceW, faceH, comp, cubeMap.format());
            const uint8_t* src = cubeMap.data().data() + i * faceSizeBytes;
            std::memcpy(faceBmp.data().data(), src, faceSizeBytes);
            try {
                preprocessor.SaveAsHDR(faceBmp, facePaths[i]);
            }
            catch (const std::exception& e) {
                Logger::GetLogger()->error("SaveFacesToDisk: face {}: {}", prefix + std::to_string(i), e.what());
            }
        }
    }

    std::shared_ptr<ITexture> TextureManager::GetTexture(const std::string& name) {
        auto it = textures_.find(name);
        if (it != textures_.end()) {
            return it->second;
        }
        Logger::GetLogger()->warn("Texture '{}' not found.", name);
        return nullptr;
    }

    std::shared_ptr<ITexture> TextureManager::LoadTexture(const std::string& name, const std::string& pathStr) {
        auto it = textures_.find(name);
        if (it != textures_.end()) {
            return it->second;
        }
        std::filesystem::path path(pathStr);
        bool isHDR = IsHDRTexture(path);
        TextureData data; // Using your separate TextureData class.
        if (!data.LoadFromFile(pathStr, true, true, isHDR)) {
            Logger::GetLogger()->error("Failed to load texture '{}' from '{}'.", name, pathStr);
            return nullptr;
        }
        bool isSRGB = DetermineSRGB(pathStr);
        TextureConfig config;
        config.isHDR = isHDR;
        config.isSRGB = isSRGB;
        try {
            auto tex = std::make_shared<OpenGLTexture>(data, config);
            textures_[name] = tex;
            Logger::GetLogger()->info("Loaded texture '{}' from '{}'. (HDR={}, sRGB={})", name, pathStr, isHDR, isSRGB);
            return tex;
        }
        catch (const std::exception& e) {
            Logger::GetLogger()->error("Exception while loading texture '{}': {}", name, e.what());
            return nullptr;
        }
    }

    void TextureManager::Clear() {
        textures_.clear();
        Logger::GetLogger()->info("Cleared all textures in TextureManager.");
    }

    std::shared_ptr<ITexture> TextureManager::CreateBRDFLUT(int width, int height, unsigned int numSamples) {
        auto& shaderManager = ShaderManager::GetInstance();
        auto computeShader = shaderManager.GetComputeShader("brdfCompute");
        if (!computeShader) {
            Logger::GetLogger()->error("Compute shader 'brdfCompute' not found!");
            return nullptr;
        }
        std::unique_ptr<ShaderStorageBuffer> ssbo;
        try {
            ssbo = std::make_unique<ShaderStorageBuffer>(1, width * height * sizeof(glm::vec2), GL_DYNAMIC_COPY);
        }
        catch (const std::exception& e) {
            Logger::GetLogger()->error("Failed to create SSBO: {}", e.what());
            return nullptr;
        }
        computeShader->Bind();
        computeShader->SetUniform("NUM_SAMPLES", numSamples);
        GLuint groupX = (width + 15) / 16;
        GLuint groupY = (height + 15) / 16;
        computeShader->Dispatch(groupX, groupY, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        computeShader->Unbind();

        GLuint brdfLUTID = 0;
        glGenTextures(1, &brdfLUTID);
        glBindTexture(GL_TEXTURE_2D, brdfLUTID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, width, height, 0, GL_RG, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        ssbo->Bind();
        glm::vec2* ptr = static_cast<glm::vec2*>(glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, width * height * sizeof(glm::vec2), GL_MAP_READ_BIT));
        if (!ptr) {
            Logger::GetLogger()->error("Failed to map SSBO for BRDF LUT.");
            ssbo->Unbind();
            glDeleteTextures(1, &brdfLUTID);
            return nullptr;
        }
        glBindTexture(GL_TEXTURE_2D, brdfLUTID);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RG, GL_FLOAT, ptr);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        ssbo->Unbind();

        // Wrap the generated texture in an ITexture implementation.
        class ExistingOpenGLTexture : public ITexture {
        public:
            ExistingOpenGLTexture(GLuint id, int w, int h) : m_TextureID(id), m_Width(w), m_Height(h) {}
            ~ExistingOpenGLTexture() override {
                if (m_TextureID) {
                    glDeleteTextures(1, &m_TextureID);
                }
            }
            void Bind(uint32_t unit) const override { glBindTextureUnit(unit, m_TextureID); }
            void Unbind(uint32_t unit) const override { glBindTextureUnit(unit, 0); }
            uint32_t GetWidth() const override { return m_Width; }
            uint32_t GetHeight() const override { return m_Height; }
            uint64_t GetBindlessHandle() const override { return 0; }
            bool IsBindless() const override { return false; }
        private:
            GLuint m_TextureID = 0;
            int m_Width = 0;
            int m_Height = 0;
        };
        return std::make_shared<ExistingOpenGLTexture>(brdfLUTID, width, height);
    }

} // namespace Graphics