#include "Resources/ShaderManager.h"
#include "Utilities/Logger.h"
#include "Graphics/Shaders/Shader.h"
#include "Graphics/Shaders/ComputeShader.h"

#include <glad/glad.h>
#include <chrono>
#include <fstream>
#include <nlohmann/json.hpp>

// Constructor remains unchanged
ShaderManager::ShaderManager(const std::string& metadataPath, const std::string& configPath)
    : m_MetadataPath(metadataPath), m_ConfigPath(configPath), m_CurrentlyBoundShader(""), m_MetaHasChanged(false)
{
    Initialize();
}

void ShaderManager::Initialize()
{
    auto logger = Logger::GetLogger();
    if (!logger) {
        std::cerr << "Logger not initialized!" << std::endl;
        return;
    }
    logger->info("Initializing ShaderManager with metadata path: '{}' and config path: '{}'.", m_MetadataPath, m_ConfigPath);

    // Load configuration and metadata
    if (!LoadMetadata()) {
        logger->warn("Failed to load metadata. Initializing with default values.");
    }
    if (!LoadConfig()) {
        logger->error("Failed to load config. Exiting initialization.");
        return;
    }

    LoadShaders();

    // Check if global metadata has changed
    if (IsGlobalMetadataChanged()) {
        m_MetaHasChanged = true;
        logger->info("Global metadata has changed. Recompiling and reloading all shaders.");

        // Recompile and reload all shaders
        for (const auto& [name, _] : m_ShadersMetadata) {
            try {
                m_Shaders[name]->ReloadShader();
                m_ShadersMetadata[name].binaryLastModified = std::filesystem::last_write_time(m_ShadersMetadata[name].binaryPath);
                logger->info("Recompiled and reloaded shader: '{}'.", name);
            }
            catch (const std::exception& e) {
                logger->error("Failed to reload shader '{}': {}", name, e.what());
            }
        }

        // Update global metadata
        m_GlobalMetadata.driverVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
        m_GlobalMetadata.openGLProfile = "core"; // Replace with actual profile if necessary
        logger->info("Updated global metadata: Driver Version = '{}', OpenGL Profile = '{}'.", m_GlobalMetadata.driverVersion, m_GlobalMetadata.openGLProfile);
    }

    if (m_MetaHasChanged) {
        if (SaveMetadata()) {
            logger->info("Metadata saved successfully.");
        }
        else {
            logger->error("Failed to save metadata.");
        }
    }
}

// Method to reload all shaders with logging
void ShaderManager::ReloadAllShaders()
{
    auto logger = Logger::GetLogger();
    logger->info("Reloading all shaders.");

    for (const auto& [name, metadata] : m_ShadersMetadata)
    {
        try {
            std::filesystem::file_time_type binaryLastModified = std::filesystem::last_write_time(metadata.binaryPath);
            std::filesystem::file_time_type sourceLastModified = std::filesystem::last_write_time(metadata.sourcePath);

            if (sourceLastModified >= binaryLastModified)
            {
                m_Shaders[name]->ReloadShader();
                m_ShadersMetadata[name].binaryLastModified = std::filesystem::last_write_time(m_ShadersMetadata[name].binaryPath);
                logger->info("Reloaded outdated shader: '{}'.", name);
            }
            else {
                logger->debug("Shader '{}' is up-to-date. No reload needed.", name);
            }
        }
        catch (const std::exception& e) {
            logger->error("Error reloading shader '{}': {}", name, e.what());
        }
    }
}

void ShaderManager::BindShader(const std::string& shaderName)
{
    auto logger = Logger::GetLogger();

    if (shaderName == m_CurrentlyBoundShader) {
        logger->debug("Shader '{}' is already bound. Skipping bind.", shaderName);
        return;
    }

    auto shaderIt = m_Shaders.find(shaderName);
    if (shaderIt == m_Shaders.end()) {
        logger->error("Shader '{}' not found. Cannot bind.", shaderName);
        return;
    }

    try {
        shaderIt->second->Bind();
        m_CurrentlyBoundShader = shaderName;
        logger->info("Shader '{}' bound successfully.", shaderName);
    }
    catch (const std::exception& e) {
        logger->error("Failed to bind shader '{}': {}", shaderName, e.what());
    }
}

void ShaderManager::LoadShaders()
{
    auto logger = Logger::GetLogger();
    logger->info("Loading shaders.");

    for (const auto& [name, metadata] : m_ShadersMetadata) {
        try {
            if (metadata.isComputeShader) {
                m_Shaders[name] = std::make_shared<ComputeShader>(metadata.sourcePath);
                logger->info("Loaded ComputeShader '{}'.", name);
            }
            else {
                m_Shaders[name] = std::make_shared<Shader>(metadata.sourcePath);
                logger->info("Loaded Shader '{}'.", name);
            }

            if (IsShaderOutdated(name)) {
                m_Shaders[name]->ReloadShader();
                m_ShadersMetadata[name].binaryLastModified = std::filesystem::last_write_time(m_ShadersMetadata[name].binaryPath);
                m_MetaHasChanged = true;
                logger->info("Shader '{}' was outdated and has been reloaded.", name);
            }
            else {
                logger->debug("Shader '{}' is up-to-date.", name);
            }
        }
        catch (const std::exception& e) {
            logger->error("Failed to load shader '{}': {}", name, e.what());
        }
    }
}

bool ShaderManager::LoadMetadata()
{
    auto logger = Logger::GetLogger();
    logger->info("Loading metadata from '{}'.", m_MetadataPath);

    std::ifstream file(m_MetadataPath);
    if (!file.is_open()) {
        logger->warn("Metadata file '{}' could not be opened. Initializing default metadata.", m_MetadataPath);
        // Initialize default metadata
        m_GlobalMetadata.driverVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
        m_GlobalMetadata.openGLProfile = "core"; // Replace with actual profile if necessary
        m_ShadersMetadata.clear();
        return false;
    }

    try {
        nlohmann::json jsonData;
        file >> jsonData;

        // Load global metadata
        if (jsonData.contains("global")) {
            m_GlobalMetadata.driverVersion = jsonData["global"].value("driver_version", "");
            m_GlobalMetadata.openGLProfile = jsonData["global"].value("openGL_profile", "");
            logger->info("Loaded global metadata: Driver Version = '{}', OpenGL Profile = '{}'.", m_GlobalMetadata.driverVersion, m_GlobalMetadata.openGLProfile);
        }

        // Load shaders metadata
        if (jsonData.contains("shaders")) {
            for (auto& [name, shaderData] : jsonData["shaders"].items()) {
                ShaderMetadata metadata;
                metadata.binaryLastModified = std::filesystem::file_time_type(std::chrono::seconds(shaderData.value("binary_last_modified", 0)));
                metadata.sourceLastModified = std::filesystem::file_time_type(std::chrono::seconds(shaderData.value("source_last_modified", 0)));
                metadata.isComputeShader = false;
                m_ShadersMetadata[name] = metadata;
                logger->info("Loaded metadata for Shader '{}'.", name);
            }
        }

        if (jsonData.contains("compute_shaders")) {
            for (auto& [name, shaderData] : jsonData["compute_shaders"].items()) {
                ShaderMetadata metadata;
                metadata.binaryLastModified = std::filesystem::file_time_type(std::chrono::seconds(shaderData.value("binary_last_modified", 0)));
                metadata.sourceLastModified = std::filesystem::file_time_type(std::chrono::seconds(shaderData.value("source_last_modified", 0)));
                metadata.isComputeShader = true;
                m_ShadersMetadata[name] = metadata;
                logger->info("Loaded metadata for ComputeShader '{}'.", name);
            }
        }
    }
    catch (const std::exception& e) {
        logger->error("Error parsing metadata file '{}': {}", m_MetadataPath, e.what());
        return false;
    }

    logger->info("Metadata loaded successfully.");
    return true;
}

bool ShaderManager::SaveMetadata()
{
    auto logger = Logger::GetLogger();
    logger->info("Saving metadata to '{}'.", m_MetadataPath);

    nlohmann::json jsonData;

    // Save global metadata
    jsonData["global"]["driver_version"] = m_GlobalMetadata.driverVersion;
    jsonData["global"]["openGL_profile"] = m_GlobalMetadata.openGLProfile;
    logger->info("Saved global metadata.");

    // Save shaders metadata
    for (const auto& [name, metadata] : m_ShadersMetadata) {
        if (metadata.isComputeShader) {
            jsonData["compute_shaders"][name]["binary_last_modified"] = std::chrono::duration_cast<std::chrono::seconds>(metadata.binaryLastModified.time_since_epoch()).count();
            jsonData["compute_shaders"][name]["source_last_modified"] = std::chrono::duration_cast<std::chrono::seconds>(metadata.sourceLastModified.time_since_epoch()).count();
            logger->info("Saved metadata for ComputeShader '{}'.", name);
        }
        else {
            jsonData["shaders"][name]["binary_last_modified"] = std::chrono::duration_cast<std::chrono::seconds>(metadata.binaryLastModified.time_since_epoch()).count();
            jsonData["shaders"][name]["source_last_modified"] = std::chrono::duration_cast<std::chrono::seconds>(metadata.sourceLastModified.time_since_epoch()).count();
            logger->info("Saved metadata for Shader '{}'.", name);
        }
    }

    std::ofstream file(m_MetadataPath);
    if (!file.is_open()) {
        logger->error("Failed to save metadata to file: '{}'.", m_MetadataPath);
        return false;
    }

    try {
        file << jsonData.dump(4); // Pretty print with 4 spaces indentation
        logger->info("Metadata saved successfully to '{}'.", m_MetadataPath);
    }
    catch (const std::exception& e) {
        logger->error("Error writing metadata to file '{}': {}", m_MetadataPath, e.what());
        return false;
    }

    m_MetaHasChanged = false;
    return true;
}

bool ShaderManager::LoadConfig()
{
    auto logger = Logger::GetLogger();
    logger->info("Loading config from '{}'.", m_ConfigPath);

    std::ifstream file(m_ConfigPath);
    if (!file.is_open()) {
        logger->error("Failed to open config file: '{}'.", m_ConfigPath);
        return false;
    }

    try {
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
                    logger->info("Shader source path '{}' exists for Shader '{}'.", metadata.sourcePath, name);
                }
                else {
                    metadata.sourceLastModified = std::filesystem::file_time_type::min();
                    logger->warn("Shader source path '{}' does not exist for Shader '{}'.", metadata.sourcePath, name);
                }

                if (std::filesystem::exists(metadata.binaryPath)) {
                    metadata.binaryLastModified = std::filesystem::last_write_time(metadata.binaryPath);
                    logger->info("Shader binary path '{}' exists for Shader '{}'.", metadata.binaryPath, name);
                }
                else {
                    metadata.binaryLastModified = std::filesystem::file_time_type::min();
                    logger->warn("Shader binary path '{}' does not exist for Shader '{}'.", metadata.binaryPath, name);
                }

                m_ShadersMetadata[name] = metadata;
                logger->info("Loaded config for Shader '{}'.", name);
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
                    logger->info("ComputeShader source path '{}' exists for ComputeShader '{}'.", metadata.sourcePath, name);
                }
                else {
                    metadata.sourceLastModified = std::filesystem::file_time_type::min();
                    logger->warn("ComputeShader source path '{}' does not exist for ComputeShader '{}'.", metadata.sourcePath, name);
                }

                if (std::filesystem::exists(metadata.binaryPath)) {
                    metadata.binaryLastModified = std::filesystem::last_write_time(metadata.binaryPath);
                    logger->info("ComputeShader binary path '{}' exists for ComputeShader '{}'.", metadata.binaryPath, name);
                }
                else {
                    metadata.binaryLastModified = std::filesystem::file_time_type::min();
                    logger->warn("ComputeShader binary path '{}' does not exist for ComputeShader '{}'.", metadata.binaryPath, name);
                }

                m_ShadersMetadata[name] = metadata;
                logger->info("Loaded config for ComputeShader '{}'.", name);
            }
        }
    }
    catch (const std::exception& e) {
        logger->error("Error parsing config file '{}': {}", m_ConfigPath, e.what());
        return false;
    }

    logger->info("Config loaded successfully.");
    return true;
}

bool ShaderManager::IsGlobalMetadataChanged()
{
    auto logger = Logger::GetLogger();
    logger->info("Checking if global metadata has changed.");

    // Get current values
    std::string currentDriverVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    std::string currentOpenGLProfile = "core"; // Replace with actual profile if necessary

    // Compare with saved metadata
    bool hasChanged = (m_GlobalMetadata.driverVersion != currentDriverVersion) ||
        (m_GlobalMetadata.openGLProfile != currentOpenGLProfile);

    if (hasChanged) {
        logger->info("Global metadata has changed.");
    }
    else {
        logger->debug("Global metadata has not changed.");
    }

    return hasChanged;
}

bool ShaderManager::IsShaderOutdated(const std::string& shaderName)
{
    auto logger = Logger::GetLogger();
    logger->debug("Checking if Shader '{}' is outdated.", shaderName);

    const auto& it = m_ShadersMetadata.find(shaderName);
    if (it == m_ShadersMetadata.end()) {
        logger->error("Shader metadata not found for shader: '{}'.", shaderName);
        return false;
    }

    const ShaderMetadata& metadata = it->second;

    try {
        // Get the last write times
        std::filesystem::file_time_type sourceLastModified = std::filesystem::last_write_time(metadata.sourcePath);
        if (!std::filesystem::exists(metadata.binaryPath))
            return true;
        std::filesystem::file_time_type binaryLastModified = std::filesystem::last_write_time(metadata.binaryPath);

        bool isOutdated = sourceLastModified >= binaryLastModified;
        if (isOutdated) {
            m_ShadersMetadata[shaderName].sourceLastModified = sourceLastModified;
            m_ShadersMetadata[shaderName].binaryLastModified = binaryLastModified;
            logger->info("Shader '{}' is outdated.", shaderName);
        }
        else {
            logger->debug("Shader '{}' is up-to-date.", shaderName);
        }

        return isOutdated;
    }
    catch (const std::exception& e) {
        logger->error("Error checking if shader '{}' is outdated: {}", shaderName, e.what());
        return false;
    }
}

std::shared_ptr<Shader> ShaderManager::GetShader(const std::string& name)
{
    auto logger = Logger::GetLogger();
    auto it = m_Shaders.find(name);
    if (it != m_Shaders.end()) {
        logger->debug("Retrieved Shader '{}'.", name);
        return std::dynamic_pointer_cast<Shader>(it->second);
    }
    logger->error("Shader '{}' not found.", name);
    return nullptr;
}

std::shared_ptr<ComputeShader> ShaderManager::GetComputeShader(const std::string& name)
{
    auto logger = Logger::GetLogger();
    auto it = m_Shaders.find(name);
    if (it != m_Shaders.end()) {
        logger->debug("Retrieved ComputeShader '{}'.", name);
        return std::dynamic_pointer_cast<ComputeShader>(it->second);
    }
    logger->error("ComputeShader '{}' not found.", name);
    return nullptr;
}