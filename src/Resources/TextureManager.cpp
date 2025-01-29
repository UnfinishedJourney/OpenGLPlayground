#include "TextureManager.h"
#include "Graphics/Textures/OpenGLTexture.h"
#include "Graphics/Textures/OpenGLCubeMapTexture.h"
#include "Graphics/Textures/OpenGLTextureArray.h" // if you have it
#include "Resources/ShaderManager.h"
#include "Graphics/Shaders/ComputeShader.h"
#include "Graphics/Buffers/ShaderStorageBuffer.h"

#include <fstream>
#include <stdexcept>
#include "Utilities/Logger.h"

TextureManager& TextureManager::GetInstance()
{
    static TextureManager instance;
    return instance;
}

TextureManager::TextureManager(const std::filesystem::path& configPath)
{
    LoadConfig(configPath);
}

bool TextureManager::LoadConfig(const std::filesystem::path& configPath)
{
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
        if (!Load2DTextures(texturesJson["2d"])) {
            return false;
        }
    }
    if (texturesJson.contains("cubeMaps")) {
        if (!LoadCubeMaps(texturesJson["cubeMaps"])) {
            return false;
        }
    }
    if (texturesJson.contains("arrays")) {
        if (!LoadTextureArrays(texturesJson["arrays"])) {
            return false;
        }
    }
    if (texturesJson.contains("computed")) {
        if (!LoadComputedTextures(texturesJson["computed"])) {
            return false;
        }
    }

    return true;
}

// Example: "2d": { "cuteDog": { "path": "assets/dog.png", "isHDR": false, "isSRGB": false } }
bool TextureManager::Load2DTextures(const nlohmann::json& json)
{
    for (auto& [name, pathOrObject] : json.items()) {
        std::string path;
        bool isHDR = false;
        bool isSRGB = false;

        // Check if user gave a simple string or an object with more info
        if (pathOrObject.is_string()) {
            path = pathOrObject.get<std::string>();
        }
        else if (pathOrObject.is_object()) {
            path = pathOrObject.value("path", "");
            isHDR = pathOrObject.value("isHDR", false);
            isSRGB = pathOrObject.value("isSRGB", false);
        }

        if (path.empty()) {
            Logger::GetLogger()->error("Texture '{}' has invalid path.", name);
            continue;
        }

        // Load texture
        auto tex = LoadTexture(name, path, isHDR, isSRGB);
        if (!tex) {
            Logger::GetLogger()->error("Failed loading 2D texture '{}'.", name);
        }
        else {
            Logger::GetLogger()->info("Loaded 2D texture '{}'.", name);
        }
    }
    return true;
}

// Example: "cubeMaps": { "pisaCube": [ "posx.png", "negx.png", ... ] }
bool TextureManager::LoadCubeMaps(const nlohmann::json& json)
{
    for (auto& [name, arrVal] : json.items()) {
        // Expect exactly 6 faces
        auto arr = arrVal.get<std::vector<std::string>>();
        if (arr.size() != 6) {
            Logger::GetLogger()->error("Cubemap '{}' must have exactly 6 faces.", name);
            continue;
        }

        std::array<std::filesystem::path, 6> faces;
        for (size_t i = 0; i < 6; i++) {
            faces[i] = arr[i];
        }

        // Example config for a cubemap
        TextureConfig cubeMapConfig;
        // Possibly read from JSON if needed, e.g. isHDR, isSRGB, etc.
        try {
            auto cubeMap = std::make_shared<OpenGLCubeMapTexture>(faces, cubeMapConfig);
            m_Textures[name] = cubeMap;
            Logger::GetLogger()->info("Loaded cube map '{}'.", name);
        }
        catch (const std::exception& e) {
            Logger::GetLogger()->error("Exception loading cube map '{}': {}", name, e.what());
        }
    }
    return true;
}

bool TextureManager::LoadTextureArrays(const nlohmann::json& json)
{
    // Implement if you have 2D texture arrays or sprite sheets.
    Logger::GetLogger()->info("LoadTextureArrays() is a stub implementation.");
    // e.g. parse { "spriteSheet": { "path": "...", "frames": 64, "gridX": 8, "gridY": 8, "isHDR": false, "isSRGB": false } }
    return true;
}

// Example "computed": { "brdfLut": { "type": "compute", "width": 256, "height": 256, "numSamples": 1024 } }
bool TextureManager::LoadComputedTextures(const nlohmann::json& json)
{
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

std::shared_ptr<ITexture> TextureManager::GetTexture(const std::string& name)
{
    auto it = m_Textures.find(name);
    if (it != m_Textures.end()) {
        return it->second;
    }
    Logger::GetLogger()->warn("Texture '{}' not found.", name);
    return nullptr;
}

std::shared_ptr<ITexture> TextureManager::LoadTexture(const std::string& name,
    const std::string& path,
    bool isHDR,
    bool isSRGB)
{

    // If already loaded, return it
    auto it = m_Textures.find(name);
    if (it != m_Textures.end()) {
        return it->second;
    }

    // Otherwise, load fresh
    TextureData data;
    if (!data.LoadFromFile(path, /*flipY*/true, /*force4Ch*/true, /*isHDR*/isHDR)) {
        Logger::GetLogger()->error("Failed to load texture '{}' from '{}'.", name, path);
        return nullptr;
    }

    TextureConfig config;
    config.isHDR = isHDR;
    config.isSRGB = isSRGB;
    // Optionally tweak config for minFilter, etc.

    try {
        auto tex = std::make_shared<OpenGLTexture>(data, config);
        m_Textures[name] = tex;
        Logger::GetLogger()->info("Loaded texture '{}' from '{}'. (HDR={}, sRGB={})",
            name, path, isHDR, isSRGB);
        return tex;
    }
    catch (const std::exception& e) {
        Logger::GetLogger()->error("Exception while loading texture '{}': {}", name, e.what());
        return nullptr;
    }
}

void TextureManager::Clear()
{
    m_Textures.clear();
    Logger::GetLogger()->info("Cleared all textures in TextureManager.");
}

// Example function to create a BRDF LUT using a compute shader
std::shared_ptr<ITexture> TextureManager::CreateBRDFLUT(int width, int height, unsigned int numSamples)
{
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

    // Map SSBO to CPU, copy to texture
    ssbo->Bind();
    glm::vec2* ptr = static_cast<glm::vec2*>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0,
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