#include "TextureManager.h"

// Include necessary headers
#include "Graphics/Textures/OpenGLTexture.h"
#include "Graphics/Textures/OpenGLCubeMapTexture.h"
#include "Graphics/Textures/OpenGLTextureArray.h" // if you have it
#include "Graphics/Textures/EnvMapProcessor.h"      // now includes SaveAsLDR & SaveFacesToDiskLDR
#include "Resources/ShaderManager.h"
#include "Graphics/Shaders/ComputeShader.h"
#include "Graphics/Buffers/ShaderStorageBuffer.h"

#include <fstream>
#include <stdexcept>
#include "Utilities/Logger.h"

// Include OpenGL and GLM headers
#include <glad/glad.h>
#include <glm/glm.hpp>

#include <stb_image.h>
#include <algorithm>
#include <filesystem>
#include <nlohmann/json.hpp>

// ----------------------------------------------------------------------------
// Small Utility Functions
// ----------------------------------------------------------------------------

// Convert a string to lowercase
std::string TextureManager::ToLower(const std::string& str) {
    std::string lowerStr = str;
    std::transform(str.begin(), str.end(), lowerStr.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return lowerStr;
}

// Check if path has .hdr / .exr / .tif / .tiff extension
bool TextureManager::IsHDRTexture(const std::filesystem::path& path) {
    static const std::vector<std::string> hdrExtensions = { ".hdr", ".exr", ".tif", ".tiff" };
    std::string ext = ToLower(path.extension().string());
    return (std::find(hdrExtensions.begin(), hdrExtensions.end(), ext) != hdrExtensions.end());
}

// Very simplistic placeholder for deciding if a texture is sRGB
bool TextureManager::DetermineSRGB(const std::string& pathStr) {
    // For now, always false or check for "_diffuse" etc.
    return false;
}

// ----------------------------------------------------------------------------
// Singleton
// ----------------------------------------------------------------------------

TextureManager& TextureManager::GetInstance() {
    static TextureManager instance;
    return instance;
}

// ----------------------------------------------------------------------------
// Constructor / Config Loading
// ----------------------------------------------------------------------------

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

    // 1) 2D Textures
    if (texturesJson.contains("2d")) {
        if (!Load2DTextures(texturesJson["2d"])) {
            return false;
        }
    }

    // 2) Cubemaps
    if (texturesJson.contains("cubeMaps")) {
        if (!LoadCubeMaps(texturesJson["cubeMaps"])) {
            return false;
        }
    }

    // 3) Arrays
    if (texturesJson.contains("arrays")) {
        if (!LoadTextureArrays(texturesJson["arrays"])) {
            return false;
        }
    }

    // 4) Computed Textures
    if (texturesJson.contains("computed")) {
        if (!LoadComputedTextures(texturesJson["computed"])) {
            return false;
        }
    }

    return true;
}

// ----------------------------------------------------------------------------
// 2D Textures
// ----------------------------------------------------------------------------

bool TextureManager::Load2DTextures(const nlohmann::json& json) {
    for (auto& [name, pathOrObject] : json.items()) {
        std::string path;

        // The JSON might be a simple string or an object with "path": "..."
        if (pathOrObject.is_string()) {
            path = pathOrObject.get<std::string>();
        }
        else if (pathOrObject.is_object()) {
            path = pathOrObject.value("path", "");
            // We could parse overrides like isSRGB, isHDR, etc.
        }

        if (path.empty()) {
            Logger::GetLogger()->error("2D Texture '{}' has invalid path.", name);
            continue;
        }

        // Create the texture immediately.
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

// ----------------------------------------------------------------------------
// Cubemaps
// ----------------------------------------------------------------------------

bool TextureManager::LoadCubeMaps(const nlohmann::json& json) {
    for (auto& [cubeMapName, arrVal] : json.items()) {
        if (!arrVal.is_array()) {
            Logger::GetLogger()->error("Cubemap '{}' should be an array of 1 or 6 paths.", cubeMapName);
            continue;
        }

        std::vector<std::string> facePaths = arrVal.get<std::vector<std::string>>();
        if (facePaths.empty()) {
            Logger::GetLogger()->error("Cubemap '{}' has an empty path array.", cubeMapName);
            continue;
        }

        // Case 1: Single path => treat as equirectangular HDR
        if (facePaths.size() == 1) {
            const std::string& equirectPath = facePaths[0];
            if (!ConvertAndLoadEquirectHDR(cubeMapName, equirectPath)) {
                Logger::GetLogger()->error("Failed to convert single-path equirect HDR '{}' for '{}'.",
                    equirectPath, cubeMapName);
            }
        }
        // Case 2: Exactly 6 paths => already a cubemap
        else if (facePaths.size() == 6) {
            std::array<std::filesystem::path, 6> faces;
            for (size_t i = 0; i < 6; i++) {
                faces[i] = facePaths[i];
            }

            // Build a configuration.
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

            // Check for HDR and sRGB flags.
            bool anyHDR = false;
            bool anySRGB = false;
            for (const auto& face : faces) {
                anyHDR = anyHDR || IsHDRTexture(face);
                anySRGB = anySRGB || DetermineSRGB(face.string());
            }
            cubeMapConfig.isHDR = anyHDR;
            cubeMapConfig.isSRGB = anySRGB;

            try {
                auto cubeMap = std::make_shared<OpenGLCubeMapTexture>(faces, cubeMapConfig);
                m_Textures[cubeMapName] = cubeMap;
                Logger::GetLogger()->info("Loaded 6-face cube map '{}'.", cubeMapName);
            }
            catch (const std::exception& e) {
                Logger::GetLogger()->error("Exception loading cube map '{}': {}", cubeMapName, e.what());
            }
        }
        else {
            Logger::GetLogger()->error("Cubemap '{}' must have 1 path (equirect) or 6. Got {}.",
                cubeMapName, facePaths.size());
        }
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
    cfg.isSRGB = false; // or check for sRGB based on naming conventions
    return cfg;
}

/**
 * @brief Convert a single equirectangular HDR path into 6 faces on disk,
 *        then load as an OpenGLCubeMapTexture.
 *
 * Now we also generate a set of LDR faces (PNG) for the skybox.
 */
bool TextureManager::ConvertAndLoadEquirectHDR(const std::string& cubeMapName, const std::string& equirectPath) {
    std::filesystem::path srcPath(equirectPath);
    if (!std::filesystem::exists(srcPath)) {
        Logger::GetLogger()->error("Equirect HDR '{}' does not exist.", equirectPath);
        return false;
    }

    std::string stem = srcPath.stem().string(); // e.g. "kloofendal_overcast_puresky_4k"
    std::filesystem::path outDir = srcPath.parent_path() / stem;

    // The environment faces (for lighting; HDR):
    std::array<std::filesystem::path, 6> envFaces = {
        outDir / "face_0.hdr",
        outDir / "face_1.hdr",
        outDir / "face_2.hdr",
        outDir / "face_3.hdr",
        outDir / "face_4.hdr",
        outDir / "face_5.hdr"
    };

    // The irradiance faces:
    std::array<std::filesystem::path, 6> irrFaces = {
        outDir / "irr_0.hdr",
        outDir / "irr_1.hdr",
        outDir / "irr_2.hdr",
        outDir / "irr_3.hdr",
        outDir / "irr_4.hdr",
        outDir / "irr_5.hdr"
    };

    // The skybox (LDR) faces:
    std::array<std::filesystem::path, 6> ldrFaces = {
        outDir / "ldr_face_0.png",
        outDir / "ldr_face_1.png",
        outDir / "ldr_face_2.png",
        outDir / "ldr_face_3.png",
        outDir / "ldr_face_4.png",
        outDir / "ldr_face_5.png"
    };

    bool alreadyConverted = std::filesystem::exists(envFaces[0]);
    bool alreadyIrr = std::filesystem::exists(irrFaces[0]);
    bool alreadyLDR = std::filesystem::exists(ldrFaces[0]);
    bool alreadyFolder = std::filesystem::exists(outDir);

    if (!alreadyFolder) {
        std::error_code ec;
        std::filesystem::create_directories(outDir, ec);
        if (ec) {
            Logger::GetLogger()->error("Cannot create directory '{}': {}", outDir.string(), ec.message());
            return false;
        }
    }

    EnvMapPreprocessor preprocessor;

    // --- ENVIRONMENT CUBEMAP FACES (HDR) ---
    if (!alreadyConverted) {
        Bitmap equirect = preprocessor.LoadTexture(srcPath);
        Bitmap vCross = preprocessor.ConvertEquirectangularMapToVerticalCross(equirect);
        Bitmap faceCube = preprocessor.ConvertVerticalCrossToCubeMapFaces(vCross);
        SaveFacesToDisk(faceCube, envFaces, "face_");
        Logger::GetLogger()->info("Created environment (HDR) faces for '{}'.", equirectPath);
    }
    else {
        Logger::GetLogger()->info("Environment faces already exist for '{}'. Skipping conversion.", equirectPath);
    }

    // --- SKYBOX FACES (LDR) ---
    if (!alreadyLDR) {
        Logger::GetLogger()->info("Generating LDR skybox faces for '{}'.", equirectPath);
        // Reuse the same vertical cross conversion from the HDR data.
        Bitmap equirect = preprocessor.LoadTexture(srcPath);
        Bitmap vCross = preprocessor.ConvertEquirectangularMapToVerticalCross(equirect);
        Bitmap faceCube = preprocessor.ConvertVerticalCrossToCubeMapFaces(vCross);
        // Save faces as LDR (PNG) instead of HDR.
        preprocessor.SaveFacesToDiskLDR(faceCube, ldrFaces, "ldr_face_");
        Logger::GetLogger()->info("Created LDR skybox faces for '{}'.", equirectPath);
    }
    else {
        Logger::GetLogger()->info("LDR skybox faces already exist for '{}'.", equirectPath);
    }

    // --- IRRADIANCE CUBEMAP FACES ---
    if (!alreadyIrr) {
        Logger::GetLogger()->info("Generating irradiance for '{}'.", equirectPath);
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

    // --- PREFILTERED (SPECULAR) CUBEMAP FACES ---
    // (Same as before.)
    std::array<std::filesystem::path, 6> prefilteredFaces0 = {
        outDir / "prefiltered_0_face_0.hdr",
        outDir / "prefiltered_0_face_1.hdr",
        outDir / "prefiltered_0_face_2.hdr",
        outDir / "prefiltered_0_face_3.hdr",
        outDir / "prefiltered_0_face_4.hdr",
        outDir / "prefiltered_0_face_5.hdr"
    };
    bool alreadyPrefiltered = std::filesystem::exists(prefilteredFaces0[0]);

    EnvMapPreprocessor preprocessorForPrefilter;
    std::vector<Bitmap> mipPrefiltered; // One Bitmap per mip level

    if (!alreadyPrefiltered) {
        Logger::GetLogger()->info("Generating prefiltered cubemap for '{}'.", equirectPath);
        Bitmap equirect = preprocessorForPrefilter.LoadTexture(srcPath);
        mipPrefiltered = preprocessorForPrefilter.ComputePrefilteredCubemap(equirect, 512, 4);
        for (size_t mip = 0; mip < mipPrefiltered.size(); mip++) {
            int faceW = mipPrefiltered[mip].w_;
            int faceH = mipPrefiltered[mip].h_;
            int comp = mipPrefiltered[mip].comp_;
            int pixelSize = comp * Bitmap::getBytesPerComponent(mipPrefiltered[mip].fmt_);
            int faceSizeBytes = faceW * faceH * pixelSize;
            for (int face = 0; face < 6; face++) {
                Bitmap faceBmp(faceW, faceH, comp, mipPrefiltered[mip].fmt_);
                const uint8_t* src = mipPrefiltered[mip].data_.data() + face * faceSizeBytes;
                std::memcpy(faceBmp.data_.data(), src, faceSizeBytes);
                std::filesystem::path outPath = outDir / ("prefiltered_" + std::to_string(mip) +
                    "_face_" + std::to_string(face) + ".hdr");
                try {
                    preprocessorForPrefilter.SaveAsHDR(faceBmp, outPath);
                }
                catch (const std::exception& e) {
                    Logger::GetLogger()->error("Error saving prefiltered face (mip {}, face {}): {}",
                        mip, face, e.what());
                }
            }
        }
        Logger::GetLogger()->info("Created prefiltered cubemap faces for '{}'.", equirectPath);
    }
    else {
        Logger::GetLogger()->info("Prefiltered cubemap faces already exist for '{}'.", equirectPath);
    }

    // --- LOAD THE PREFILTERED (SPECULAR) CUBEMAP AS AN OPENGL TEXTURE ---
    size_t mipLevels = 0;
    while (true) {
        std::filesystem::path testFace = outDir / ("prefiltered_" + std::to_string(mipLevels) + "_face_0.hdr");
        if (!std::filesystem::exists(testFace))
            break;
        mipLevels++;
    }
    //if (mipLevels == 0) {
    //    Logger::GetLogger()->error("No mip levels found for prefiltered cubemap '{}'.", equirectPath);
    //}
    //else {
    //    std::vector<std::array<std::filesystem::path, 6>> mipFacesPaths(mipLevels);
    //    for (size_t mip = 0; mip < mipLevels; mip++) {
    //        std::array<std::filesystem::path, 6> faces;
    //        for (int face = 0; face < 6; face++) {
    //            faces[face] = outDir / ("prefiltered_" + std::to_string(mip) +
    //                "_face_" + std::to_string(face) + ".hdr");
    //        }
    //        mipFacesPaths[mip] = faces;
    //    }
    //    TextureConfig prefilterConfig = MakeSomeCubeMapConfig(true);
    //    prefilterConfig.generateMips = false; // Already computed
    //    try {
    //        auto prefilteredCube = std::make_shared<OpenGLCubeMapTexture>(mipFacesPaths, prefilterConfig);
    //        std::string prefilterName = cubeMapName + "_prefiltered";
    //        m_Textures[prefilterName] = prefilteredCube;
    //        Logger::GetLogger()->info("Loaded prefiltered cubemap '{}'.", prefilterName);
    //    }
    //    catch (const std::exception& e) {
    //        Logger::GetLogger()->error("Exception creating prefiltered cubemap '{}': {}", cubeMapName, e.what());
    //    }
    //}

    // --- LOAD THE MAIN ENVIRONMENT CUBEMAP (HDR version for lighting) ---
    TextureConfig envCfg = MakeSomeCubeMapConfig(true);
    try {
        auto cubeMap = std::make_shared<OpenGLCubeMapTexture>(envFaces, envCfg);
        m_Textures[cubeMapName] = cubeMap;
    }
    catch (const std::exception& e) {
        Logger::GetLogger()->error("Exception creating env cubemap '{}': {}", cubeMapName, e.what());
        return false;
    }

    // --- LOAD THE IRRADIANCE CUBEMAP ---
    TextureConfig irrCfg = MakeSomeCubeMapConfig(true);
    try {
        auto irrCube = std::make_shared<OpenGLCubeMapTexture>(irrFaces, irrCfg);
        std::string irrName = cubeMapName + "_irr";
        m_Textures[irrName] = irrCube;
        Logger::GetLogger()->info("Created irradiance cubemap '{}'.", irrName);
    }
    catch (const std::exception& e) {
        Logger::GetLogger()->warn("Failed to create irradiance cubemap for '{}': {}", cubeMapName, e.what());
    }

    // --- LOAD THE SKYBOX CUBEMAP (LDR version for display) ---
    // Build a texture config for the skybox that is not HDR.
    TextureConfig skyboxCfg;
    skyboxCfg.internalFormat = GL_SRGB8_ALPHA8; // 8-bit sRGB
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
        std::string skyboxName = cubeMapName + "_skybox";
        m_Textures[skyboxName] = skyboxCube;
        Logger::GetLogger()->info("Loaded skybox (LDR) cubemap '{}'.", skyboxName);
    }
    catch (const std::exception& e) {
        Logger::GetLogger()->error("Exception creating skybox cubemap '{}': {}", cubeMapName, e.what());
    }

    return true;
}

// ----------------------------------------------------------------------------
// SaveFacesToDisk (existing version)
// ----------------------------------------------------------------------------

void TextureManager::SaveFacesToDisk(const Bitmap& cubeMap,
    const std::array<std::filesystem::path, 6>& facePaths,
    const std::string& prefix) {
    if (cubeMap.d_ != 6) {
        Logger::GetLogger()->error("SaveFacesToDisk: expected 6 faces in depth, found d_={}", cubeMap.d_);
        return;
    }

    int faceW = cubeMap.w_;
    int faceH = cubeMap.h_;
    int comp = cubeMap.comp_;
    eBitmapFormat fmt = cubeMap.fmt_;

    int pixelSize = comp * Bitmap::getBytesPerComponent(fmt);
    int faceSizeBytes = faceW * faceH * pixelSize;

    EnvMapPreprocessor prep; // for SaveAsHDR

    for (int i = 0; i < 6; i++) {
        Bitmap faceBmp(faceW, faceH, comp, fmt);
        const uint8_t* src = cubeMap.data_.data() + i * faceSizeBytes;
        std::memcpy(faceBmp.data_.data(), src, faceSizeBytes);
        try {
            prep.SaveAsHDR(faceBmp, facePaths[i]);
        }
        catch (const std::exception& e) {
            Logger::GetLogger()->error("SaveFacesToDisk: face {}: {}", prefix + std::to_string(i), e.what());
        }
    }
}

// ----------------------------------------------------------------------------
// Texture Arrays (stub)
// ----------------------------------------------------------------------------

bool TextureManager::LoadTextureArrays(const nlohmann::json& json) {
    Logger::GetLogger()->info("LoadTextureArrays() is a stub implementation.");
    return true;
}

// ----------------------------------------------------------------------------
// Computed (BRDF LUT, etc.)
// ----------------------------------------------------------------------------

bool TextureManager::LoadComputedTextures(const nlohmann::json& json) {
    for (auto& [name, info] : json.items()) {
        std::string type = info.value("type", "");
        if (type == "compute") {
            int width = info.value("width", 256);
            int height = info.value("height", 256);
            unsigned int smpls = info.value("numSamples", 1024);
            auto tex = CreateBRDFLUT(width, height, smpls);
            if (tex) {
                m_Textures[name] = tex;
                Logger::GetLogger()->info("Computed texture '{}' created (BRDF LUT).", name);
            }
        }
        else {
            Logger::GetLogger()->error("Unknown computed texture type '{}' for '{}'.", type, name);
        }
    }
    return true;
}

// ----------------------------------------------------------------------------
// Public Get / Load / Clear
// ----------------------------------------------------------------------------

std::shared_ptr<ITexture> TextureManager::GetTexture(const std::string& name) {
    auto it = m_Textures.find(name);
    if (it != m_Textures.end()) {
        return it->second;
    }
    Logger::GetLogger()->warn("Texture '{}' not found.", name);
    return nullptr;
}

std::shared_ptr<ITexture> TextureManager::LoadTexture(const std::string& name, const std::string& pathStr) {
    // If already loaded, return it
    auto it = m_Textures.find(name);
    if (it != m_Textures.end()) {
        return it->second;
    }

    std::filesystem::path path(pathStr);
    bool isHDR = IsHDRTexture(path);

    TextureData data;
    if (!data.LoadFromFile(pathStr, /*flipY*/ true, /*force4Ch*/ true, isHDR)) {
        Logger::GetLogger()->error("Failed to load texture '{}' from '{}'.", name, pathStr);
        return nullptr;
    }

    bool isSRGB = DetermineSRGB(pathStr);

    TextureConfig config;
    config.isHDR = isHDR;
    config.isSRGB = isSRGB;

    try {
        std::shared_ptr<ITexture> tex;
        if (isHDR) {
            tex = std::make_shared<OpenGLTexture>(data, config);
        }
        else {
            tex = std::make_shared<OpenGLTexture>(data, config);
        }
        m_Textures[name] = tex;
        Logger::GetLogger()->info("Loaded texture '{}' from '{}'. (HDR={}, sRGB={})",
            name, pathStr, isHDR, isSRGB);
        return tex;
    }
    catch (const std::exception& e) {
        Logger::GetLogger()->error("Exception while loading texture '{}': {}", name, e.what());
        return nullptr;
    }
}

void TextureManager::Clear() {
    m_Textures.clear();
    Logger::GetLogger()->info("Cleared all textures in TextureManager.");
}

// ----------------------------------------------------------------------------
// BRDF LUT (Compute Example)
// ----------------------------------------------------------------------------

std::shared_ptr<ITexture> TextureManager::CreateBRDFLUT(int width, int height, unsigned int numSamples) {
    auto& shaderManager = ShaderManager::GetInstance();
    auto computeShader = shaderManager.GetComputeShader("brdfCompute");
    if (!computeShader) {
        Logger::GetLogger()->error("Compute shader 'brdfCompute' not found!");
        return nullptr;
    }

    std::unique_ptr<Graphics::ShaderStorageBuffer> ssbo;
    try {
        ssbo = std::make_unique<Graphics::ShaderStorageBuffer>(
            /*binding point=*/1,
            width * height * sizeof(glm::vec2),
            GL_DYNAMIC_COPY
        );
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
    glm::vec2* ptr = static_cast<glm::vec2*>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER,
            0,
            width * height * sizeof(glm::vec2),
            GL_MAP_READ_BIT)
        );
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

    class ExistingOpenGLTexture : public ITexture {
    public:
        ExistingOpenGLTexture(GLuint id, int w, int h)
            : m_TextureID(id), m_Width(w), m_Height(h) {}
        ~ExistingOpenGLTexture() override {
            if (m_TextureID) {
                glDeleteTextures(1, &m_TextureID);
            }
        }
        void Bind(uint32_t unit) const override { glBindTextureUnit(unit, m_TextureID); }
        void Unbind(uint32_t unit) const override { glBindTextureUnit(unit, 0); }
        uint32_t GetWidth()  const override { return m_Width; }
        uint32_t GetHeight() const override { return m_Height; }
        uint64_t GetBindlessHandle() const override { return 0; }
        bool     IsBindless()        const override { return false; }
    private:
        GLuint m_TextureID = 0;
        int    m_Width = 0;
        int    m_Height = 0;
    };

    return std::make_shared<ExistingOpenGLTexture>(brdfLUTID, width, height);
}