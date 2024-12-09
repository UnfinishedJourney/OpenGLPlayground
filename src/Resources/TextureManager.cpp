#include "Resources/TextureManager.h"
#include "Utilities/Logger.h"
#include "Graphics/Shaders/ComputeShader.h"
#include "Resources/ShaderManager.h"
#include "Graphics/Textures/Texture2D.h"
#include "Graphics/Buffers/ShaderStorageBuffer.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <stdexcept>

TextureManager& TextureManager::GetInstance() {
    static TextureManager instance;
    return instance;
}

TextureManager::TextureManager() {
    m_TexturePaths = {
        {"cuteDog", "../assets/cute_dog.png"},
        {"heightmap", "../assets/heightmap.png"},
        {"duckDiffuse", "../assets/rubber_duck/textures/Duck_baseColor.png"},
        {"damagedHelmet1", "../assets/DamagedHelmet/glTF/Default_albedo.jpg"},
        {"damagedHelmet2", "../assets/DamagedHelmet/glTF/Default_AO.jpg"},
        {"damagedHelmet3", "../assets/DamagedHelmet/glTF/Default_emissive.jpg"}
    };

    m_CubeMapTexturePaths = {
        {"pisa", {
            "../assets/cube/pisa/pisa_posx.png",
            "../assets/cube/pisa/pisa_negx.png",
            "../assets/cube/pisa/pisa_posy.png",
            "../assets/cube/pisa/pisa_negy.png",
            "../assets/cube/pisa/pisa_posz.png",
            "../assets/cube/pisa/pisa_negz.png"
        }}
    };

    InitializeBRDFLUT();
}

std::filesystem::path TextureManager::GetTexturePath(const std::string& textureName) const {
    auto it = m_TexturePaths.find(textureName);
    if (it != m_TexturePaths.end()) {
        return it->second;
    }
    else {
        Logger::GetLogger()->error("Texture name '{}' not found in texture path mappings.", textureName);
        return {};
    }
}

std::array<std::filesystem::path, 6> TextureManager::GetCubeMapPaths(const std::string& cubeMapName) const {
    auto it = m_CubeMapTexturePaths.find(cubeMapName);
    if (it != m_CubeMapTexturePaths.end()) {
        return it->second;
    }
    else {
        Logger::GetLogger()->error("CubeMap name '{}' not found in cube map texture path mappings.", cubeMapName);
        return {};
    }
}

std::shared_ptr<Texture2D> TextureManager::GetTexture2D(const std::string& textureName) {
    std::lock_guard<std::mutex> lock(m_Mutex); 

    auto it = m_Texture2Ds.find(textureName);
    if (it != m_Texture2Ds.end()) {
        return it->second;
    }

    auto texturePath = GetTexturePath(textureName);
    if (texturePath.empty() || !std::filesystem::exists(texturePath)) {
        Logger::GetLogger()->error("Texture file '{}' does not exist.", texturePath.string());
        return nullptr;
    }

    try {
        auto texture = std::make_shared<Texture2D>(texturePath);
        m_Texture2Ds.emplace(textureName, texture);
        return texture;
    }
    catch (const std::exception& e) {
        Logger::GetLogger()->error("Exception while loading Texture2D '{}': {}", textureName, e.what());
        return nullptr;
    }
}

std::shared_ptr<CubeMapTexture> TextureManager::GetCubeMapTexture(const std::string& cubeMapName) {
    std::lock_guard<std::mutex> lock(m_Mutex);

    auto it = m_CubeMapTextures.find(cubeMapName);
    if (it != m_CubeMapTextures.end()) {
        return it->second;
    }

    auto facePaths = GetCubeMapPaths(cubeMapName);
    if (facePaths.empty()) {
        Logger::GetLogger()->error("Cube map face paths for '{}' are empty.", cubeMapName);
        return nullptr;
    }

    for (const auto& path : facePaths) {
        if (!std::filesystem::exists(path)) {
            Logger::GetLogger()->error("Cube map face file '{}' does not exist.", path.string());
            return nullptr;
        }
    }

    try {
        auto cubeMap = std::make_shared<CubeMapTexture>(facePaths);
        m_CubeMapTextures.emplace(cubeMapName, cubeMap);
        return cubeMap;
    }
    catch (const std::exception& e) {
        Logger::GetLogger()->error("Exception while loading CubeMapTexture '{}': {}", cubeMapName, e.what());
        return nullptr;
    }
}

std::shared_ptr<Texture2D> TextureManager::GetHeightMap(const std::string& heightMapName) {
    std::lock_guard<std::mutex> lock(m_Mutex); 

    auto it = m_HeightMaps.find(heightMapName);
    if (it != m_HeightMaps.end()) {
        return it->second;
    }

    auto texturePath = GetTexturePath(heightMapName);
    if (texturePath.empty() || !std::filesystem::exists(texturePath)) {
        Logger::GetLogger()->error("Heightmap file '{}' does not exist.", texturePath.string());
        return nullptr;
    }

    try {
        auto heightMap = std::make_shared<Texture2D>(texturePath);
        m_HeightMaps.emplace(heightMapName, heightMap);
        return heightMap;
    }
    catch (const std::exception& e) {
        Logger::GetLogger()->error("Exception while loading HeightMap '{}': {}", heightMapName, e.what());
        return nullptr;
    }
}

void TextureManager::RegisterTexture2D(const std::string& name, std::shared_ptr<Texture2D> texture) {
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (m_Texture2Ds.find(name) != m_Texture2Ds.end()) {
        Logger::GetLogger()->warn("Texture2D with name '{}' already exists. Overwriting.", name);
    }
    m_Texture2Ds[name] = texture;
    Logger::GetLogger()->info("Texture2D '{}' registered.", name);
}

void TextureManager::InitializeBRDFLUT(int width, int height, unsigned int numSamples) {
    Logger::GetLogger()->info("Initializing BRDF LUT with size {}x{} and {} samples.", width, height, numSamples);

    // Retrieve the compute shader responsible for generating the BRDF LUT
    auto& shaderManager = ShaderManager::GetInstance();
    auto computeShader = shaderManager.GetComputeShader("brdfCompute");
    if (!computeShader) {
        Logger::GetLogger()->error("Compute shader 'brdfCompute' not found!");
        return;
    }

    // Create a Shader Storage Buffer Object (SSBO) using your custom ShaderStorageBuffer class
    std::unique_ptr<ShaderStorageBuffer> ssbo;
    try {
        ssbo = std::make_unique<ShaderStorageBuffer>(10, width * height * sizeof(glm::vec2), GL_DYNAMIC_COPY);
        //one was conflicting with my lights binding point....
    }
    catch (const std::exception& e) {
        Logger::GetLogger()->error("Failed to create ShaderStorageBuffer: {}", e.what());
        return;
    }

    // Bind the compute shader and set the required uniform
    computeShader->Bind();
    computeShader->SetUniform("NUM_SAMPLES", numSamples);

    // Calculate the number of workgroups based on the local workgroup size (16x16)
    GLuint groupX = (width + 15) / 16;
    GLuint groupY = (height + 15) / 16;

    Logger::GetLogger()->info("Dispatching compute shader with group sizes: ({}, {})", groupX, groupY);
    computeShader->Dispatch(groupX, groupY, 1);

    // Ensure memory coherence before reading from the buffer
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    computeShader->Unbind();

    // Generate and configure the BRDF LUT texture
    GLuint brdfLUTTextureID;
    glGenTextures(1, &brdfLUTTextureID);
    if (brdfLUTTextureID == 0) {
        Logger::GetLogger()->error("Failed to create BRDF LUT texture.");
        return;
    }

    glBindTexture(GL_TEXTURE_2D, brdfLUTTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, width, height, 0, GL_RG, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Bind the SSBO before mapping
    ssbo->Bind();

    // Map the SSBO to read the computed BRDF data
    glm::vec2* ptr = static_cast<glm::vec2*>(glMapBufferRange(
        GL_SHADER_STORAGE_BUFFER,
        0,
        width * height * sizeof(glm::vec2),
        GL_MAP_READ_BIT
    ));

    if (ptr) {
        // Optional: Log the first few values for debugging
        for (int i = 0; i < 10 && i < width * height; ++i) {
            Logger::GetLogger()->info("BRDF LUT [{}]: ({}, {})", i, ptr[i].x, ptr[i].y);
        }

        // Upload the data from the SSBO to the texture
        glBindTexture(GL_TEXTURE_2D, brdfLUTTextureID);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RG, GL_FLOAT, ptr);
        glBindTexture(GL_TEXTURE_2D, 0);

        // Unmap the buffer after reading
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    }
    else {
        Logger::GetLogger()->error("Failed to map the BRDF SSBO buffer.");
    }

    // Unbind the SSBO
    ssbo->Unbind();

    // Wrap the BRDF LUT texture using your Texture2D class
    std::shared_ptr<Texture2D> brdfLUTTexture;
    try {
        brdfLUTTexture = std::make_shared<Texture2D>(brdfLUTTextureID, width, height, GL_RG32F);
    }
    catch (const std::exception& e) {
        Logger::GetLogger()->error("Failed to create Texture2D for BRDF LUT: {}", e.what());
        glDeleteTextures(1, &brdfLUTTextureID);
        return;
    }

    // Register the texture for later use
    RegisterTexture2D("brdfLUT", brdfLUTTexture);

    Logger::GetLogger()->info("BRDF LUT initialized and registered as 'brdfLUT'.");
}

void TextureManager::Clear() {
    std::lock_guard<std::mutex> lock(m_Mutex);

    m_Texture2Ds.clear();
    m_CubeMapTextures.clear();
    m_HeightMaps.clear();

    Logger::GetLogger()->info("Cleared all textures from TextureManager.");
}