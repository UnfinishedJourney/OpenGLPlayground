#include "TextureManager.h"
#include "Graphics/Textures/OpenGLTexture.h"
#include "Graphics/Textures/OpenGLCubeMapTexture.h"
#include "Graphics/Textures/OpenGLTextureArray.h"
#include "Utilities/Logger.h"
#include "Resources/ShaderManager.h"
#include "Graphics/Shaders/ComputeShader.h"
#include "Graphics/Buffers/ShaderStorageBuffer.h"
#include "Graphics/Textures/TextureData.h"
#include <fstream>
#include <stdexcept>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <stb_image_write.h>

TextureManager& TextureManager::GetInstance() {
    static TextureManager instance;
    return instance;
}

bool TextureManager::LoadConfig(const std::filesystem::path& configPath) {
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (!std::filesystem::exists(configPath)) {
        Logger::GetLogger()->error("Texture config file '{}' does not exist.", configPath.string());
        return false;
    }

    std::ifstream file(configPath);
    if (!file) {
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

    const auto& texturesJson = jsonData["textures"];
    if (texturesJson.contains("2d") && !Load2DTextures(texturesJson["2d"])) return false;
    if (texturesJson.contains("cubeMaps") && !LoadCubeMaps(texturesJson["cubeMaps"])) return false;
    if (texturesJson.contains("arrays") && !LoadTextureArrays(texturesJson["arrays"])) return false;
    if (texturesJson.contains("computed") && !LoadComputedTextures(texturesJson["computed"])) return false;

    return true;
}

bool TextureManager::Load2DTextures(const nlohmann::json& json) {
    for (auto& [name, pathVal] : json.items()) {
        std::string path = pathVal.get<std::string>();
        TextureData data;
        if (!data.LoadFromFile(path)) {
            Logger::GetLogger()->error("Failed to load 2D texture '{}': {}", name, path);
            continue;
        }

        TextureConfig config;
        try {
            auto tex = std::make_shared<OpenGLTexture>(data, config);
            m_Textures[name] = tex;
            Logger::GetLogger()->info("Loaded 2D texture '{}'.", name);
        }
        catch (const std::exception& e) {
            Logger::GetLogger()->error("Exception loading 2D texture '{}': {}", name, e.what());
        }
    }
    return true;
}

std::shared_ptr<ITexture> TextureManager::LoadTexture(const std::string& name, const std::string& path) {
    std::lock_guard<std::mutex> lock(m_Mutex);

    // Check if the texture is already loaded
    auto it = m_Textures.find(name);
    if (it != m_Textures.end()) {
        return it->second;
    }

    // Verify that the file exists
    if (!std::filesystem::exists(path)) {
        Logger::GetLogger()->error("Texture '{}' not found at path '{}'.", name, path);
        return nullptr;
    }

    // Load texture data
    TextureData data;
    if (!data.LoadFromFile(path)) {
        Logger::GetLogger()->error("Failed to load texture '{}' from '{}'.", name, path);
        return nullptr;
    }

    // Configure texture parameters as needed
    TextureConfig config; // Customize as required

    try {
        // Create an OpenGL texture instance
        auto tex = std::make_shared<OpenGLTexture>(data, config);
        m_Textures[name] = tex;
        Logger::GetLogger()->info("Loaded texture '{}' from '{}'.", name, path);
        return tex;
    }
    catch (const std::exception& e) {
        Logger::GetLogger()->error("Exception while loading texture '{}': {}", name, e.what());
        return nullptr;
    }
}

bool TextureManager::LoadCubeMaps(const nlohmann::json& json) {
    for (auto& [name, arrVal] : json.items()) {
        auto arr = arrVal.get<std::vector<std::string>>();
        if (arr.size() != 6) {
            Logger::GetLogger()->error("Cube map '{}' must have exactly 6 faces.", name);
            continue;
        }
        std::array<std::filesystem::path, 6> faces;
        for (size_t i = 0; i < 6; i++) faces[i] = arr[i];

        TextureConfig config;
        try {
            auto cubeMap = std::make_shared<OpenGLCubeMapTexture>(faces, config);
            m_Textures[name] = cubeMap;
            Logger::GetLogger()->info("Loaded cube map '{}'.", name);
        }
        catch (const std::exception& e) {
            Logger::GetLogger()->error("Exception loading cube map '{}': {}", name, e.what());
        }
    }
    return true;
}

bool TextureManager::LoadTextureArrays(const nlohmann::json& json) {
    for (auto& [name, arrVal] : json.items()) {
        auto arr = arrVal.get<std::vector<std::string>>();
        if (arr.empty()) {
            Logger::GetLogger()->error("Texture array '{}' is empty.", name);
            continue;
        }

        TextureConfig config;
        try {
            auto texArray = std::make_shared<OpenGLTextureArray>(arr, config);
            m_Textures[name] = texArray;
            Logger::GetLogger()->info("Loaded texture array '{}'.", name);
        }
        catch (const std::exception& e) {
            Logger::GetLogger()->error("Exception loading texture array '{}': {}", name, e.what());
        }
    }
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
                m_Textures[name] = tex;
                Logger::GetLogger()->info("Computed texture '{}' created.", name);
            }
        }
        else {
            Logger::GetLogger()->error("Unknown computed texture type '{}' for '{}'.", type, name);
        }
    }
    return true;
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
        ssbo = std::make_unique<ShaderStorageBuffer>(10, width * height * sizeof(glm::vec2), GL_DYNAMIC_COPY);
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

    GLuint brdfLUTID;
    glGenTextures(1, &brdfLUTID);
    glBindTexture(GL_TEXTURE_2D, brdfLUTID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, width, height, 0, GL_RG, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    ssbo->Bind();
    glm::vec2* ptr = (glm::vec2*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, width * height * sizeof(glm::vec2), GL_MAP_READ_BIT);
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

    // Wrap into a basic ITexture
    class ExistingOpenGLTexture : public ITexture {
    public:
        ExistingOpenGLTexture(GLuint id, int w, int h)
            : m_TextureID(id), m_Width(w), m_Height(h) {}
        ~ExistingOpenGLTexture() {
            if (m_TextureID) glDeleteTextures(1, &m_TextureID);
        }
        void Bind(uint32_t unit) const override { glBindTextureUnit(unit, m_TextureID); }
        void Unbind(uint32_t unit) const override { glBindTextureUnit(unit, 0); }
        uint32_t GetWidth() const override { return m_Width; }
        uint32_t GetHeight() const override { return m_Height; }
        uint64_t GetBindlessHandle() const override { return 0; }
        bool IsBindless() const override { return false; }

    private:
        GLuint m_TextureID;
        int m_Width, m_Height;
    };

    return std::make_shared<ExistingOpenGLTexture>(brdfLUTID, width, height);
}

std::shared_ptr<ITexture> TextureManager::GetTexture(const std::string& name) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    auto it = m_Textures.find(name);
    if (it != m_Textures.end()) return it->second;
    Logger::GetLogger()->warn("Texture '{}' not found.", name);
    return nullptr;
}

void TextureManager::Clear() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Textures.clear();
    Logger::GetLogger()->info("Cleared all textures from TextureManager.");
}

bool TextureManager::SaveTexture(const std::string& textureName, const std::filesystem::path& outputFile)
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    // 1. Look up the texture in our map
    auto it = m_Textures.find(textureName);
    if (it == m_Textures.end()) {
        Logger::GetLogger()->error("SaveTexture failed: Texture '{}' not found.", textureName);
        return false;
    }

    // 2. Cast to OpenGLTexture so we can access the native texture ID
    auto glTexture = std::dynamic_pointer_cast<OpenGLTexture>(it->second);
    if (!glTexture) {
        Logger::GetLogger()->error("SaveTexture failed: Texture '{}' is not an OpenGLTexture.", textureName);
        return false;
    }

    // 3. Retrieve the texture ID and dimensions
    GLuint textureID = glTexture->GetTextureID();
    int width = static_cast<int>(glTexture->GetWidth());
    int height = static_cast<int>(glTexture->GetHeight());

    // 4. Bind the texture and read back data via glGetTexImage
    glBindTexture(GL_TEXTURE_2D, textureID);

    // By default, let's assume it's an RGBA8 texture.
    // We'll read it back as GL_RGBA, GL_UNSIGNED_BYTE.
    // If your texture's internal format is different (e.g., GL_RG32F),
    // you need to adjust the read parameters and how you convert it to 8-bit.
    std::vector<unsigned char> pixels(width * height * 4);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    glBindTexture(GL_TEXTURE_2D, 0);

    // 5. Save with stb_image_write. For an RGBA8, we can do:
    //    stbi_write_png  (for PNG)
    //    stbi_write_jpg  (for JPEG)
    //    stbi_write_tga  (for TGA)
    //    stbi_write_bmp  (for BMP)
    //    stbi_write_hdr  (for HDR) [but that takes float data, not bytes]

    // For PNG:
    int channels = 4;
    int strideInBytes = width * channels;

    if (!stbi_write_png(outputFile.string().c_str(),
        width,
        height,
        channels,
        pixels.data(),
        strideInBytes))
    {
        Logger::GetLogger()->error("Failed to write texture '{}' to file '{}'.", textureName, outputFile.string());
        return false;
    }

    Logger::GetLogger()->info("Saved texture '{}' to '{}'.", textureName, outputFile.string());
    return true;
}