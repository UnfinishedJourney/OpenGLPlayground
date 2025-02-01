#include "TextureManager.h"

// Include necessary headers
#include "Graphics/Textures/OpenGLTexture.h"
#include "Graphics/Textures/OpenGLCubeMapTexture.h"
#include "Graphics/Textures/OpenGLTextureArray.h" // if you have it
#include "Graphics/Textures/EnvMapProcessor.h"
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

    // 1) 2D
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

    // 4) Computed
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
            // We could parse overrides like isSRGB, isHDR, etc. if we wanted
        }

        if (path.empty()) {
            Logger::GetLogger()->error("2D Texture '{}' has invalid path.", name);
            continue;
        }

        // Create the texture (immediately)
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

bool TextureManager::LoadCubeMaps(const nlohmann::json& json)
{
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

        // Case 1: Single path => treat as equirect HDR
        if (facePaths.size() == 1) {
            const std::string& equirectPath = facePaths[0];
            if (!ConvertAndLoadEquirectHDR(cubeMapName, equirectPath)) {
                Logger::GetLogger()->error("Failed to convert single-path equirect HDR '{}' for '{}'.",
                    equirectPath, cubeMapName);
            }
        }
        // Case 2: Exactly 6 => already a cubemap
        else if (facePaths.size() == 6) {
            std::array<std::filesystem::path, 6> faces;
            for (size_t i = 0; i < 6; i++) {
                faces[i] = facePaths[i];
            }

            // Build the config
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

            // Check if any face is HDR
            bool anyHDR = false;
            bool anySRGB = false;
            for (const auto& face : faces) {
                anyHDR = anyHDR || IsHDRTexture(face);
                anySRGB = anySRGB || DetermineSRGB(face.string());
            }
            cubeMapConfig.isHDR = anyHDR;
            cubeMapConfig.isSRGB = anySRGB;

            // Create the cubemap now
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

TextureConfig TextureManager::MakeSomeCubeMapConfig(bool isHDR)
{
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
    cfg.isSRGB = false; // or check
    return cfg;
}

/**
 * @brief Convert a single equirect HDR path into 6 faces on disk, then load as OpenGLCubeMapTexture.
 */
bool TextureManager::ConvertAndLoadEquirectHDR(const std::string& cubeMapName,
                                               const std::string& equirectPath)
{
    std::filesystem::path srcPath(equirectPath);
    if (!std::filesystem::exists(srcPath))
    {
        Logger::GetLogger()->error("Equirect HDR '{}' does not exist.", equirectPath);
        return false;
    }

    std::string stem = srcPath.stem().string(); // e.g. "kloofendal_overcast_puresky_4k"
    std::filesystem::path outDir = srcPath.parent_path() / stem;

    // The environment faces:
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

    bool alreadyConverted = std::filesystem::exists(envFaces[0]); 
    bool alreadyIrr       = std::filesystem::exists(irrFaces[0]);
    bool alreadyFolder = std::filesystem::exists(outDir);

    if (!alreadyFolder)
    {
        std::error_code ec;
        std::filesystem::create_directories(outDir, ec);
        if (ec)
        {
            Logger::GetLogger()->error("Cannot create directory '{}': {}",
                outDir.string(), ec.message());
            return false;
        }
    }


    if (!alreadyConverted)
    {
        // 1) Load equirect
        EnvMapPreprocessor preprocessor;
        Bitmap equirect = preprocessor.LoadTexture(srcPath);

        // 2) Convert to vertical cross
        Bitmap vCross = preprocessor.ConvertEquirectangularMapToVerticalCross(equirect);

        // 3) verticalCross -> 6-face cube
        Bitmap faceCube = preprocessor.ConvertVerticalCrossToCubeMapFaces(vCross);

        // Save each face_*
        SaveFacesToDisk(faceCube, envFaces, /*prefix=*/"face_");
        Logger::GetLogger()->info("Created environment faces for '{}'.", equirectPath);
    }
    else
    {
        Logger::GetLogger()->info("Env faces already exist for '{}'. Skipping conversion.", equirectPath);
    }

    // Next, check if we have the irradiance faces
    if (!alreadyIrr)
    {
        Logger::GetLogger()->info("Generating IRRADIANCE for '{}'.", equirectPath);

        EnvMapPreprocessor preprocessor;
        // 1) Load equirect again
        Bitmap equirect = preprocessor.LoadTexture(srcPath);

        // 2) Convolve to get smaller blurred equirect, e.g. 256x128 with 1024 samples
        //    The size is up to you. 128x64 also works, etc.
        Bitmap equirectIrr = preprocessor.ComputeIrradianceEquirect(equirect, 256, 128, 1024);

        // 3) Convert that irradiance equirect to vertical cross
        Bitmap vCrossIrr = preprocessor.ConvertEquirectangularMapToVerticalCross(equirectIrr);

        // 4) Cross -> 6-face
        Bitmap faceCubeIrr = preprocessor.ConvertVerticalCrossToCubeMapFaces(vCrossIrr);

        // 5) Save as irr_0.hdr..irr_5.hdr
        SaveFacesToDisk(faceCubeIrr, irrFaces, /*prefix=*/"irr_");
        Logger::GetLogger()->info("Created irradiance faces for '{}'.", equirectPath);
    }
    else
    {
        Logger::GetLogger()->info("Irradiance faces already exist for '{}'.", equirectPath);
    }

    // Finally, load the environment faces as an OpenGLCubeMapTexture (the “main” environment).
    TextureConfig envCfg = MakeSomeCubeMapConfig(/*HDR=*/true);
    try
    {
        auto cubeMap = std::make_shared<OpenGLCubeMapTexture>(envFaces, envCfg);
        m_Textures[cubeMapName] = cubeMap;
    }
    catch (const std::exception& e)
    {
        Logger::GetLogger()->error("Exception creating env cubemap '{}': {}", cubeMapName, e.what());
        return false;
    }

    // Optionally also load the IRR cube map and store it as "cubeMapName_irr" or something
    // so you can get it later with GetTexture("overcastSky_irr").
    TextureConfig irrCfg = MakeSomeCubeMapConfig(/*HDR=*/true);
    try
    {
        auto irrCube = std::make_shared<OpenGLCubeMapTexture>(irrFaces, irrCfg);
        std::string irrName = cubeMapName + "_irr";
        m_Textures[irrName] = irrCube;
        Logger::GetLogger()->info("Created irradiance cubemap '{}'.", irrName);
    }
    catch (const std::exception& e)
    {
        Logger::GetLogger()->warn("Failed to create irradiance cubemap for '{}': {}", cubeMapName, e.what());
        // Not necessarily fatal
    }

    return true;
}

void TextureManager::SaveFacesToDisk(const Bitmap& cubeMap,
    const std::array<std::filesystem::path, 6>& facePaths,
    const std::string& prefix)
{
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

    for (int i = 0; i < 6; i++)
    {
        Bitmap faceBmp(faceW, faceH, comp, fmt);
        const uint8_t* src = cubeMap.data_.data() + i * faceSizeBytes;
        std::memcpy(faceBmp.data_.data(), src, faceSizeBytes);

        try {
            prep.SaveAsHDR(faceBmp, facePaths[i]);
        }
        catch (const std::exception& e)
        {
            Logger::GetLogger()->error("SaveFacesToDisk: face {}: {}", prefix + std::to_string(i), e.what());
        }
    }
}

// ----------------------------------------------------------------------------
// Arrays (stub)
// ----------------------------------------------------------------------------

bool TextureManager::LoadTextureArrays(const nlohmann::json& json) {
    // No changes from your old code if you had it
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

    // Determine HDR based on file extension
    bool isHDR = IsHDRTexture(path);

    // Load texture data (RGBA float or 8-bit)
    TextureData data;
    if (!data.LoadFromFile(pathStr, /*flipY*/true, /*force4Ch*/true, isHDR)) {
        Logger::GetLogger()->error("Failed to load texture '{}' from '{}'.", name, pathStr);
        return nullptr;
    }

    // Determine sRGB from naming convention or placeholder
    bool isSRGB = DetermineSRGB(pathStr);

    // Build config
    TextureConfig config;
    config.isHDR = isHDR;
    config.isSRGB = isSRGB;
    // e.g. config.minFilter = GL_LINEAR_MIPMAP_LINEAR; etc. (if you want)

    try {
        std::shared_ptr<ITexture> tex;
        if (isHDR) {
            // 32-bit float
            tex = std::make_shared<OpenGLTexture>(data, config);
        }
        else {
            // 8-bit
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

    std::unique_ptr<ShaderStorageBuffer> ssbo;
    try {
        ssbo = std::make_unique<ShaderStorageBuffer>(
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

    // Create an empty RG32F texture
    GLuint brdfLUTID = 0;
    glGenTextures(1, &brdfLUTID);
    glBindTexture(GL_TEXTURE_2D, brdfLUTID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, width, height, 0, GL_RG, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Map SSBO to CPU and copy to that texture
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

    // Return as a simple ITexture
    class ExistingOpenGLTexture : public ITexture {
    public:
        ExistingOpenGLTexture(GLuint id, int w, int h)
            : m_TextureID(id), m_Width(w), m_Height(h) {}

        ~ExistingOpenGLTexture() override {
            if (m_TextureID) {
                glDeleteTextures(1, &m_TextureID);
            }
        }

        void Bind(uint32_t unit) const override {
            glBindTextureUnit(unit, m_TextureID);
        }
        void Unbind(uint32_t unit) const override {
            glBindTextureUnit(unit, 0);
        }

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