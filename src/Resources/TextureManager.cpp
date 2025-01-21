#include "TextureManager.h"
#include "Graphics/Textures/OpenGLTexture.h"
#include "Graphics/Textures/OpenGLCubeMapTexture.h"
// ... plus your other includes for arrays, etc.

#include <fstream>
#include <stdexcept>

// If you use a Logger or just std::cout:
#include "Utilities/Logger.h"     // example
//#include <iostream>             // alternative

TextureManager& TextureManager::GetInstance()
{
    static TextureManager instance;
    return instance;
}

bool TextureManager::LoadConfig(const std::filesystem::path& configPath)
{
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

    // The root object expects: { "textures": { "2d": {}, "cubeMaps": {}, ... } }
    if (!jsonData.contains("textures")) {
        Logger::GetLogger()->error("JSON config missing 'textures' object.");
        return false;
    }
    auto texturesJson = jsonData["textures"];

    // 2D
    if (texturesJson.contains("2d")) {
        if (!Load2DTextures(texturesJson["2d"])) {
            return false;
        }
    }

    // Cubemaps
    if (texturesJson.contains("cubeMaps")) {
        if (!LoadCubeMaps(texturesJson["cubeMaps"])) {
            return false;
        }
    }

    // (Optionally) 2D Arrays
    if (texturesJson.contains("arrays")) {
        if (!LoadTextureArrays(texturesJson["arrays"])) {
            return false;
        }
    }

    // (Optionally) computed textures
    if (texturesJson.contains("computed")) {
        if (!LoadComputedTextures(texturesJson["computed"])) {
            return false;
        }
    }

    return true;
}

bool TextureManager::Load2DTextures(const nlohmann::json& json)
{
    // The format is:
    // "2d": {
    //    "cuteDog": "path/to/cute_dog.png",
    //    "duckDiffuse": "path/to/Duck_baseColor.png"
    // }
    for (auto& [name, pathVal] : json.items()) {
        std::string path = pathVal.get<std::string>();

        // Load data
        TextureData data;
        if (!data.LoadFromFile(path)) {
            Logger::GetLogger()->error("Failed loading 2D texture '{}': {}", name, path);
            continue; // skip
        }

        // Create the OpenGLTexture
        TextureConfig config; // default config
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

bool TextureManager::LoadCubeMaps(const nlohmann::json& json)
{
    // The format is:
    // "cubeMaps": {
    //    "pisaCube": [
    //       "pisa_posx.png",
    //       "pisa_negx.png",
    //       "pisa_posy.png",
    //       "pisa_negy.png",
    //       "pisa_posz.png",
    //       "pisa_negz.png"
    //    ]
    // }

    for (auto& [name, arrVal] : json.items()) {
        // Expect exactly 6 faces
        auto arr = arrVal.get<std::vector<std::string>>();
        if (arr.size() != 6) {
            Logger::GetLogger()->error("Cubemap '{}' must have exactly 6 faces", name);
            continue;
        }

        std::array<std::filesystem::path, 6> faces;
        for (size_t i = 0; i < 6; i++) {
            faces[i] = arr[i];
        }

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

bool TextureManager::LoadTextureArrays(const nlohmann::json& json)
{
    // Implement if you have 2D array textures or sprite sheets, etc.
    Logger::GetLogger()->info("LoadTextureArrays() not implemented for brevity here.");
    return true;
}

bool TextureManager::LoadComputedTextures(const nlohmann::json& json)
{
    // e.g., "computed": {
    //   "brdfLUT": { "type": "compute", "width":256, "height":256, "numSamples":1024 }
    // }
    Logger::GetLogger()->info("LoadComputedTextures() not implemented for brevity here.");
    return true;
}

std::shared_ptr<ITexture> TextureManager::CreateBRDFLUT(int width, int height, unsigned int numSamples)
{
    // Example of a specialized compute-based BRDF LUT creation.
    // Omitted for brevity; reference your existing code if needed.
    return nullptr;
}

std::shared_ptr<ITexture> TextureManager::LoadTexture(const std::string& name, const std::string& path)
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    // If already in the map, return it.
    auto it = m_Textures.find(name);
    if (it != m_Textures.end()) {
        return it->second;
    }

    // Otherwise, load it fresh
    TextureData data;
    if (!data.LoadFromFile(path)) {
        Logger::GetLogger()->error("Failed to load texture '{}' from '{}'.", name, path);
        return nullptr;
    }
    TextureConfig config;
    try {
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

std::shared_ptr<ITexture> TextureManager::GetTexture(const std::string& name)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    auto it = m_Textures.find(name);
    if (it != m_Textures.end()) {
        return it->second;
    }
    Logger::GetLogger()->warn("Texture '{}' not found.", name);
    return nullptr;
}

void TextureManager::Clear()
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Textures.clear();
    Logger::GetLogger()->info("Cleared all textures in TextureManager.");
}