#include "Resources/ShaderManager.h"
#include <glad/glad.h>
#include <chrono>

ShaderManager::ShaderManager(const std::string& metadataPath, const std::string& configPath)
    : m_MetadataPath(metadataPath), m_ConfigPath(configPath), m_MetaHasChanged(false)
{
    Initialize();
}

void ShaderManager::Initialize()
{
    // Load configuration and metadata
    if (!LoadMetadata()) {
        std::cerr << "Failed to load metadata. Initializing with default values." << std::endl;
    }
    if (!LoadConfig()) {
        std::cerr << "Failed to load config. Exiting." << std::endl;
        return;
    }

    LoadShaders();

    // Check if global metadata has changed
    if (IsGlobalMetadataChanged()) {
        m_MetaHasChanged = true;

        // Recompile and reload all shaders
        for (const auto& [name, _] : m_ShadersMetadata) {
            m_Shaders[name]->ReloadShader();
            m_ShadersMetadata[name].binaryLastModified = std::filesystem::last_write_time(m_ShadersMetadata[name].binaryPath);
        }

        // Update global metadata
        m_GlobalMetadata.driverVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
        m_GlobalMetadata.openGLProfile = "core"; // Replace with actual profile if necessary
    }

    if (m_MetaHasChanged) {
        SaveMetadata();
    }
}

//need to also check includes
void ShaderManager::ReloadAllShaders()
{
    for (const auto& [name, metadata] : m_ShadersMetadata)
    {
        std::filesystem::file_time_type binaryLastModified = std::filesystem::last_write_time(metadata.binaryPath);
        std::filesystem::file_time_type sourceLastModified = std::filesystem::last_write_time(metadata.sourcePath);
        if (sourceLastModified >= binaryLastModified)
        {
            m_Shaders[name]->ReloadShader();
            m_ShadersMetadata[name].binaryLastModified = std::filesystem::last_write_time(m_ShadersMetadata[name].binaryPath);
        }

    }
}

void ShaderManager::LoadShaders()
{
    for (const auto& [name, metadata] : m_ShadersMetadata) {
        if (metadata.isComputeShader) {
            m_Shaders[name] = std::make_shared<ComputeShader>(metadata.sourcePath);
        }
        else {
            m_Shaders[name] = std::make_shared<Shader>(metadata.sourcePath);
        }
        if (IsShaderOutdated(name)) {
            m_Shaders[name]->ReloadShader();
            m_ShadersMetadata[name].binaryLastModified = std::filesystem::last_write_time(m_ShadersMetadata[name].binaryPath);
            m_MetaHasChanged = true;
        }
    }
}

bool ShaderManager::LoadMetadata()
{
    std::ifstream file(m_MetadataPath);
    if (!file.is_open()) {
        // Initialize default metadata
        m_GlobalMetadata.driverVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
        m_GlobalMetadata.openGLProfile = "core"; // Replace with actual profile if necessary
        m_ShadersMetadata.clear();
        return false;
    }

    nlohmann::json jsonData;
    file >> jsonData;

    // Load global metadata
    if (jsonData.contains("global")) {
        m_GlobalMetadata.driverVersion = jsonData["global"].value("driver_version", "");
        m_GlobalMetadata.openGLProfile = jsonData["global"].value("openGL_profile", "");
    }

    // Load shaders metadata
    if (jsonData.contains("shaders")) {
        for (auto& [name, shaderData] : jsonData["shaders"].items()) {
            ShaderMetadata metadata;
            metadata.binaryLastModified = std::filesystem::file_time_type(std::chrono::seconds(shaderData.value("binary_last_modified", 0)));
            metadata.sourceLastModified = std::filesystem::file_time_type(std::chrono::seconds(shaderData.value("source_last_modified", 0)));
            metadata.isComputeShader = false;
            m_ShadersMetadata[name] = metadata;
        }
    }

    if (jsonData.contains("compute_shaders")) {
        for (auto& [name, shaderData] : jsonData["compute_shaders"].items()) {
            ShaderMetadata metadata;
            metadata.binaryLastModified = std::filesystem::file_time_type(std::chrono::seconds(shaderData.value("binary_last_modified", 0)));
            metadata.sourceLastModified = std::filesystem::file_time_type(std::chrono::seconds(shaderData.value("source_last_modified", 0)));
            metadata.isComputeShader = true;
            m_ShadersMetadata[name] = metadata;
        }
    }

    return true;
}

bool ShaderManager::SaveMetadata()
{
    nlohmann::json jsonData;

    // Save global metadata
    jsonData["global"]["driver_version"] = m_GlobalMetadata.driverVersion;
    jsonData["global"]["openGL_profile"] = m_GlobalMetadata.openGLProfile;

    // Save shaders metadata
    for (const auto& [name, metadata] : m_ShadersMetadata) {
        if (metadata.isComputeShader) {
            jsonData["compute_shaders"][name]["binary_last_modified"] = std::chrono::duration_cast<std::chrono::seconds>(metadata.binaryLastModified.time_since_epoch()).count();
            jsonData["compute_shaders"][name]["source_last_modified"] = std::chrono::duration_cast<std::chrono::seconds>(metadata.sourceLastModified.time_since_epoch()).count();
        }
        else {
            jsonData["shaders"][name]["binary_last_modified"] = std::chrono::duration_cast<std::chrono::seconds>(metadata.binaryLastModified.time_since_epoch()).count();
            jsonData["shaders"][name]["source_last_modified"] = std::chrono::duration_cast<std::chrono::seconds>(metadata.sourceLastModified.time_since_epoch()).count();
        }
    }

    std::ofstream file(m_MetadataPath);
    if (!file.is_open()) {
        std::cerr << "Failed to save metadata to file: " << m_MetadataPath << std::endl;
        return false;
    }

    file << jsonData.dump(4); // Pretty print with 4 spaces indentation
    m_MetaHasChanged = false;
    return true;
}

bool ShaderManager::LoadConfig()
{
    std::ifstream file(m_ConfigPath);
    if (!file.is_open()) {
        std::cerr << "Failed to open config file: " << m_ConfigPath << std::endl;
        return false;
    }

    nlohmann::json jsonData;
    file >> jsonData;

    // Load shaders paths from config
    if (jsonData.contains("shaders")) {
        for (auto& [name, shaderData] : jsonData["shaders"].items()) {
            ShaderMetadata metadata;
            metadata.sourcePath = shaderData.value("source_path", "");
            metadata.binaryPath = shaderData.value("binary_path", "");
            metadata.isComputeShader = false;

            // Initialize last modified times
            if (std::filesystem::exists(metadata.sourcePath)) {
                metadata.sourceLastModified = std::filesystem::last_write_time(metadata.sourcePath);
            }
            else {
                metadata.sourceLastModified = std::filesystem::file_time_type::min();
            }

            if (std::filesystem::exists(metadata.binaryPath)) {
                metadata.binaryLastModified = std::filesystem::last_write_time(metadata.binaryPath);
            }
            else {
                metadata.binaryLastModified = std::filesystem::file_time_type::min();
            }

            m_ShadersMetadata[name] = metadata;
        }
    }

    if (jsonData.contains("compute_shaders")) {
        for (auto& [name, shaderData] : jsonData["compute_shaders"].items()) {
            ShaderMetadata metadata;
            metadata.sourcePath = shaderData.value("source_path", "");
            metadata.binaryPath = shaderData.value("binary_path", "");
            metadata.isComputeShader = true;

            // Initialize last modified times
            if (std::filesystem::exists(metadata.sourcePath)) {
                metadata.sourceLastModified = std::filesystem::last_write_time(metadata.sourcePath);
            }
            else {
                metadata.sourceLastModified = std::filesystem::file_time_type::min();
            }

            if (std::filesystem::exists(metadata.binaryPath)) {
                metadata.binaryLastModified = std::filesystem::last_write_time(metadata.binaryPath);
            }
            else {
                metadata.binaryLastModified = std::filesystem::file_time_type::min();
            }

            m_ShadersMetadata[name] = metadata;
        }
    }

    return true;
}

bool ShaderManager::IsGlobalMetadataChanged()
{
    // Get current values
    std::string currentDriverVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    std::string currentOpenGLProfile = "core"; // Replace with actual profile if necessary

    // Compare with saved metadata
    return m_GlobalMetadata.driverVersion != currentDriverVersion ||
        m_GlobalMetadata.openGLProfile != currentOpenGLProfile;
}

bool ShaderManager::IsShaderOutdated(const std::string& shaderName)
{
    const auto& it = m_ShadersMetadata.find(shaderName);
    if (it == m_ShadersMetadata.end()) {
        std::cerr << "Shader metadata not found for shader: " << shaderName << std::endl;
        return false;
    }

    const ShaderMetadata& metadata = it->second;

    // Get the last write times
    std::filesystem::file_time_type sourceLastModified = std::filesystem::last_write_time(metadata.sourcePath);
    if (!std::filesystem::exists(metadata.binaryPath))
        return true;
    std::filesystem::file_time_type binaryLastModified = std::filesystem::last_write_time(metadata.binaryPath);

    bool isOutdated = sourceLastModified >= binaryLastModified;
    if (isOutdated) {
        m_ShadersMetadata[shaderName].sourceLastModified = sourceLastModified;
        m_ShadersMetadata[shaderName].binaryLastModified = binaryLastModified;
    }

    return isOutdated;
}

std::shared_ptr<Shader> ShaderManager::GetShader(const std::string& name)
{
    auto it = m_Shaders.find(name);
    if (it != m_Shaders.end()) {
        return std::dynamic_pointer_cast<Shader>(it->second);
    }
    std::cerr << "Shader not found: " << name << std::endl;
    return nullptr;
}

std::shared_ptr<ComputeShader> ShaderManager::GetComputeShader(const std::string& name)
{
    auto it = m_Shaders.find(name);
    if (it != m_Shaders.end()) {
        return std::dynamic_pointer_cast<ComputeShader>(it->second);
    }
    std::cerr << "Compute shader not found: " << name << std::endl;
    return nullptr;
}