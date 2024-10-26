#include "ShaderManager.h"
#include "Utilities/Logger.h"

#include <glad/glad.h>
#include <fstream>
#include <chrono>
#include <stdexcept>

ShaderManager::ShaderManager(const std::filesystem::path& metadataPath, const std::filesystem::path& configPath)
    : m_MetadataPath(metadataPath), m_ConfigPath(configPath)
{
    Initialize();
}

void ShaderManager::Initialize()
{
    auto logger = Logger::GetLogger();
    logger->info("Initializing ShaderManager with metadata path: '{}' and config path: '{}'.",
        m_MetadataPath.string(), m_ConfigPath.string());

    if (!LoadMetadata()) {
        logger->warn("Failed to load metadata. Initializing with default values.");
    }
    if (!LoadConfig()) {
        logger->error("Failed to load config. Exiting initialization.");
        return;
    }

    LoadShaders();

    if (IsGlobalMetadataChanged()) {
        logger->info("Global metadata has changed. Recompiling and reloading all shaders.");
        ReloadAllShaders();
        m_GlobalMetadata.driverVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
        m_GlobalMetadata.openGLProfile = "core"; // Adjust as necessary
        SaveMetadata();
    }
}

void ShaderManager::LoadShaders()
{
    auto logger = Logger::GetLogger();
    logger->info("Loading shaders.");

    for (const auto& [name, metadata] : m_ShadersMetadata) {
        try {
            std::shared_ptr<BaseShader> shader;
            if (metadata.isComputeShader) {
                shader = std::make_shared<ComputeShader>(metadata.sourcePath, metadata.binaryPath);
                logger->info("Loaded ComputeShader '{}'.", name);
            }
            else {
                shader = std::make_shared<Shader>(metadata.sourcePath, metadata.binaryPath);
                logger->info("Loaded Shader '{}'.", name);
            }

            // Thread-safe insertion
            {
                std::unique_lock lock(m_ShaderMutex);
                m_Shaders[name] = shader;
            }

            if (IsShaderOutdated(name)) {
                shader->ReloadShader();
                m_ShadersMetadata[name].binaryLastModified = std::filesystem::last_write_time(metadata.binaryPath);
                SaveMetadata();
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

void ShaderManager::ReloadAllShaders()
{
    auto logger = Logger::GetLogger();
    logger->info("Reloading all shaders.");

    for (const auto& [name, shader] : m_Shaders) {
        try {
            shader->ReloadShader();
            m_ShadersMetadata[name].binaryLastModified = std::filesystem::last_write_time(m_ShadersMetadata[name].binaryPath);
            logger->info("Reloaded shader '{}'.", name);
        }
        catch (const std::exception& e) {
            logger->error("Error reloading shader '{}': {}", name, e.what());
        }
    }

    SaveMetadata();
}

void ShaderManager::BindShader(std::string_view shaderName)
{
    auto logger = Logger::GetLogger();

    if (shaderName == m_CurrentlyBoundShader) {
        logger->debug("Shader '{}' is already bound.", shaderName);
        return;
    }

    std::shared_ptr<BaseShader> shader;
    {
        std::shared_lock lock(m_ShaderMutex);
        auto it = m_Shaders.find(std::string(shaderName));
        if (it == m_Shaders.end()) {
            logger->error("Shader '{}' not found.", shaderName);
            return;
        }
        shader = it->second;
    }

    try {
        shader->Bind();
        m_CurrentlyBoundShader = std::string(shaderName);
        logger->info("Shader '{}' bound successfully.", std::string(shaderName));
    }
    catch (const std::exception& e) {
        logger->error("Failed to bind shader '{}': {}", std::string(shaderName), e.what());
    }
         
    //EnqueueGLCommand([shader, shaderNameStr = std::string(shaderName), this, logger]() {
    //    try {
    //        shader->Bind();
    //        m_CurrentlyBoundShader = shaderNameStr;
    //        logger->info("Shader '{}' bound successfully.", shaderNameStr);
    //    }
    //    catch (const std::exception& e) {
    //        logger->error("Failed to bind shader '{}': {}", shaderNameStr, e.what());
    //    }
    //    });
}

std::shared_ptr<Shader> ShaderManager::GetShader(std::string_view name)
{
    std::shared_lock lock(m_ShaderMutex);
    auto it = m_Shaders.find(std::string(name));
    if (it != m_Shaders.end()) {
        return std::dynamic_pointer_cast<Shader>(it->second);
    }
    Logger::GetLogger()->error("Shader '{}' not found.", name);
    return nullptr;
}

std::shared_ptr<ComputeShader> ShaderManager::GetComputeShader(std::string_view name)
{
    std::shared_lock lock(m_ShaderMutex);
    auto it = m_Shaders.find(std::string(name));
    if (it != m_Shaders.end()) {
        return std::dynamic_pointer_cast<ComputeShader>(it->second);
    }
    Logger::GetLogger()->error("ComputeShader '{}' not found.", name);
    return nullptr;
}

std::shared_ptr<BaseShader> ShaderManager::GetCurrentlyBoundShader() const
{
    std::shared_lock lock(m_ShaderMutex);
    auto it = m_Shaders.find(m_CurrentlyBoundShader);
    if (it != m_Shaders.end()) {
        return it->second;
    }
    return nullptr;
}

bool ShaderManager::LoadMetadata()
{
    auto logger = Logger::GetLogger();
    logger->info("Loading metadata from '{}'.", m_MetadataPath.string());

    if (!std::filesystem::exists(m_MetadataPath)) {
        logger->warn("Metadata file '{}' does not exist.", m_MetadataPath.string());
        return false;
    }

    std::ifstream file(m_MetadataPath);
    if (!file) {
        logger->error("Failed to open metadata file '{}'.", m_MetadataPath.string());
        return false;
    }

    try {
        nlohmann::json jsonData;
        file >> jsonData;

        m_GlobalMetadata.driverVersion = jsonData["global"]["driver_version"].get<std::string>();
        m_GlobalMetadata.openGLProfile = jsonData["global"]["openGL_profile"].get<std::string>();

        for (const auto& [name, shaderData] : jsonData["shaders"].items()) {
            ShaderMetadata metadata;
            metadata.isComputeShader = shaderData["is_compute_shader"].get<bool>();
            metadata.sourcePath = shaderData["source_path"].get<std::string>();
            metadata.binaryPath = shaderData["binary_path"].get<std::string>();
            m_ShadersMetadata[name] = metadata;
        }
    }
    catch (const std::exception& e) {
        logger->error("Error parsing metadata file '{}': {}", m_MetadataPath.string(), e.what());
        return false;
    }

    logger->info("Metadata loaded successfully.");
    return true;
}

bool ShaderManager::SaveMetadata() const
{
    auto logger = Logger::GetLogger();
    logger->info("Saving metadata to '{}'.", m_MetadataPath.string());

    nlohmann::json jsonData;

    jsonData["global"]["driver_version"] = m_GlobalMetadata.driverVersion;
    jsonData["global"]["openGL_profile"] = m_GlobalMetadata.openGLProfile;

    for (const auto& [name, metadata] : m_ShadersMetadata) {
        jsonData["shaders"][name]["is_compute_shader"] = metadata.isComputeShader;
        jsonData["shaders"][name]["source_path"] = metadata.sourcePath.string();
        jsonData["shaders"][name]["binary_path"] = metadata.binaryPath.string();
        jsonData["shaders"][name]["source_last_modified"] = std::chrono::duration_cast<std::chrono::seconds>(
            metadata.sourceLastModified.time_since_epoch()).count();
        jsonData["shaders"][name]["binary_last_modified"] = std::chrono::duration_cast<std::chrono::seconds>(
            metadata.binaryLastModified.time_since_epoch()).count();
    }

    std::ofstream file(m_MetadataPath);
    if (!file) {
        logger->error("Failed to open metadata file '{}' for writing.", m_MetadataPath.string());
        return false;
    }

    try {
        file << jsonData.dump(4);
    }
    catch (const std::exception& e) {
        logger->error("Error writing metadata to file '{}': {}", m_MetadataPath.string(), e.what());
        return false;
    }

    logger->info("Metadata saved successfully.");
    return true;
}

bool ShaderManager::LoadConfig()
{
    auto logger = Logger::GetLogger();
    logger->info("Loading config from '{}'.", m_ConfigPath.string());

    if (!std::filesystem::exists(m_ConfigPath)) {
        logger->error("Config file '{}' does not exist.", m_ConfigPath.string());
        return false;
    }

    std::ifstream file(m_ConfigPath);
    if (!file) {
        logger->error("Failed to open config file '{}'.", m_ConfigPath.string());
        return false;
    }

    try {
        nlohmann::json jsonData;
        file >> jsonData;

        for (const auto& [name, shaderData] : jsonData["shaders"].items()) {
            ShaderMetadata metadata;
            metadata.isComputeShader = shaderData.value("is_compute_shader", false);
            metadata.sourcePath = shaderData["source_path"].get<std::string>();
            metadata.binaryPath = shaderData["binary_path"].get<std::string>();

            if (std::filesystem::exists(metadata.sourcePath)) {
                metadata.sourceLastModified = std::filesystem::last_write_time(metadata.sourcePath);
            }
            else {
                metadata.sourceLastModified = std::filesystem::file_time_type::min();
                logger->warn("Shader source path '{}' does not exist for Shader '{}'.", metadata.sourcePath.string(), name);
            }

            if (std::filesystem::exists(metadata.binaryPath)) {
                metadata.binaryLastModified = std::filesystem::last_write_time(metadata.binaryPath);
            }
            else {
                metadata.binaryLastModified = std::filesystem::file_time_type::min();
                logger->warn("Shader binary path '{}' does not exist for Shader '{}'.", metadata.binaryPath.string(), name);
            }

            m_ShadersMetadata[name] = metadata;
        }
    }
    catch (const std::exception& e) {
        logger->error("Error parsing config file '{}': {}", m_ConfigPath.string(), e.what());
        return false;
    }

    logger->info("Config loaded successfully.");
    return true;
}

bool ShaderManager::IsGlobalMetadataChanged() const
{
    auto currentDriverVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    auto currentOpenGLProfile = "core"; // Adjust as necessary

    return m_GlobalMetadata.driverVersion != currentDriverVersion ||
        m_GlobalMetadata.openGLProfile != currentOpenGLProfile;
}

bool ShaderManager::IsShaderOutdated(const std::string& shaderName) const
{
    auto it = m_ShadersMetadata.find(shaderName);
    if (it == m_ShadersMetadata.end()) {
        Logger::GetLogger()->error("Shader metadata not found for shader '{}'.", shaderName);
        return false;
    }

    const auto& metadata = it->second;

    if (!std::filesystem::exists(metadata.binaryPath)) {
        return true;
    }

    auto sourceLastModified = std::filesystem::last_write_time(metadata.sourcePath);
    auto binaryLastModified = std::filesystem::last_write_time(metadata.binaryPath);

    return sourceLastModified > binaryLastModified;
}

//void ShaderManager::EnqueueGLCommand(std::function<void()> command)
//{
//    std::lock_guard lock(m_GLCommandQueueMutex);
//    m_GLCommandQueue.push(std::move(command));
//}
//
//void ShaderManager::ExecuteGLCommands()
//{
//    std::queue<std::function<void()>> commands;
//    {
//        std::lock_guard lock(m_GLCommandQueueMutex);
//        std::swap(commands, m_GLCommandQueue);
//    }
//
//    while (!commands.empty()) {
//        commands.front()();
//        commands.pop();
//    }
//}