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

    for (auto& [name, metadata] : m_ShadersMetadata) {
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

            m_Shaders[name] = shader;

            if (IsShaderOutdated(name)) {
                shader->ReloadShader();
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

    for (auto& [name, shader] : m_Shaders) {
        try {
            shader->ReloadShader();
            logger->info("Reloaded shader '{}'.", name);
        }
        catch (const std::exception& e) {
            logger->error("Error reloading shader '{}': {}", name, e.what());
        }
    }
}

void ShaderManager::BindShader(std::string_view shaderName)
{
    auto logger = Logger::GetLogger();

    if (shaderName == m_CurrentlyBoundShader) {
        logger->debug("Shader '{}' is already bound.", shaderName);
        return;
    }

    std::shared_ptr<BaseShader> shader;
    auto it = m_Shaders.find(std::string(shaderName));
    if (it == m_Shaders.end()) {
        logger->error("Shader '{}' not found.", shaderName);
        return;
    }
    shader = it->second;

    try {
        shader->Bind();
        m_CurrentlyBoundShader = std::string(shaderName);
        logger->debug("Shader '{}' bound successfully.", shaderName);
    }
    catch (const std::exception& e) {
        logger->error("Failed to bind shader '{}': {}", shaderName, e.what());
    }
}

std::shared_ptr<Shader> ShaderManager::GetShader(std::string_view name)
{
    auto it = m_Shaders.find(std::string(name));
    if (it != m_Shaders.end()) {
        return std::dynamic_pointer_cast<Shader>(it->second);
    }
    Logger::GetLogger()->error("Shader '{}' not found.", name);
    return nullptr;
}

std::shared_ptr<ComputeShader> ShaderManager::GetComputeShader(std::string_view name)
{
    auto it = m_Shaders.find(std::string(name));
    if (it != m_Shaders.end()) {
        return std::dynamic_pointer_cast<ComputeShader>(it->second);
    }
    Logger::GetLogger()->error("ComputeShader '{}' not found.", name);
    return nullptr;
}

std::shared_ptr<BaseShader> ShaderManager::GetCurrentlyBoundShader() const
{
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
            metadata.isComputeShader = shaderData.value("is_compute_shader", false);
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

    // Get the latest modification time of the source file and included files
    std::unordered_set<std::string> processedFiles;
    auto latestSourceTime = GetLatestShaderModificationTime(metadata.sourcePath, processedFiles);

    auto binaryLastModified = std::filesystem::last_write_time(metadata.binaryPath);

    return latestSourceTime > binaryLastModified;
}

std::filesystem::file_time_type ShaderManager::GetLatestShaderModificationTime(
    const std::filesystem::path& sourcePath,
    std::unordered_set<std::string>& processedFiles) const
{
    auto logger = Logger::GetLogger();

    // Normalize and check if the file has already been processed
    auto sourcePathCanonical = std::filesystem::canonical(sourcePath).string();
    if (processedFiles.find(sourcePathCanonical) != processedFiles.end()) {
        return std::filesystem::file_time_type::min();
    }
    processedFiles.insert(sourcePathCanonical);

    // Get the modification time of the current source file
    std::filesystem::file_time_type latestTime = std::filesystem::last_write_time(sourcePath);

    // Read the shader source file
    std::ifstream file(sourcePath);
    if (!file) {
        logger->error("Failed to open shader file: {}", sourcePath.string());
        throw std::runtime_error("Failed to open shader file: " + sourcePath.string());
    }

    std::string line;
    bool inBlockComment = false;

    while (std::getline(file, line)) {
        std::string trimmedLine = line;
        // Remove leading whitespace
        trimmedLine.erase(0, trimmedLine.find_first_not_of(" \t"));

        // Handle block comments
        if (inBlockComment) {
            size_t endComment = trimmedLine.find("*/");
            if (endComment != std::string::npos) {
                inBlockComment = false;
                trimmedLine = trimmedLine.substr(endComment + 2);
            }
            else {
                continue; // Skip line inside block comment
            }
        }

        // Check for start of block comment
        size_t startBlockComment = trimmedLine.find("/*");
        if (startBlockComment != std::string::npos) {
            inBlockComment = true;
            trimmedLine = trimmedLine.substr(0, startBlockComment);
        }

        // Remove single-line comments
        size_t singleLineComment = trimmedLine.find("//");
        if (singleLineComment != std::string::npos) {
            trimmedLine = trimmedLine.substr(0, singleLineComment);
        }

        // Trim whitespace again
        trimmedLine.erase(0, trimmedLine.find_first_not_of(" \t"));
        trimmedLine.erase(trimmedLine.find_last_not_of(" \t\r\n") + 1);

        // Skip empty lines
        if (trimmedLine.empty()) {
            continue;
        }

        // Process includes
        if (trimmedLine.find("#include") == 0) {
            size_t start = trimmedLine.find_first_of("\"<");
            size_t end = trimmedLine.find_first_of("\">", start + 1);
            if (start == std::string::npos || end == std::string::npos) {
                logger->error("Invalid #include syntax in file '{}': {}", sourcePath.string(), line);
                throw std::runtime_error("Invalid #include syntax in file '" + sourcePath.string() + "': " + line);
            }

            std::string includePathStr = trimmedLine.substr(start + 1, end - start - 1);
            std::filesystem::path includePath = sourcePath.parent_path() / includePathStr;

            // Normalize the path
            includePath = std::filesystem::weakly_canonical(includePath);

            // Check if the file exists
            if (!std::filesystem::exists(includePath)) {
                logger->warn("Included file does not exist: {}", includePath.string());
                continue;
            }

            // Recursively get the latest modification time
            auto includeLatestTime = GetLatestShaderModificationTime(includePath, processedFiles);

            if (includeLatestTime > latestTime) {
                latestTime = includeLatestTime;
            }
        }
    }

    return latestTime;
}