#include "ShaderManager.h"

// Standard library
#include <fstream>
#include <chrono>
#include <stdexcept>

// External
#include <nlohmann/json.hpp>

// Your engine/logging
#include "Utilities/Logger.h"

// If needed, include <glad/glad.h> or ensure it's included in the PCH

ShaderManager::ShaderManager(const std::filesystem::path& metadataPath,
    const std::filesystem::path& configPath)
    : m_MetadataPath(metadataPath)
    , m_ConfigPath(configPath)
{
    // Example default uniform/storage blocks
    m_UniformBlockBindings = {
        {"FrameData", FRAME_DATA_BINDING_POINT}
        // Add more uniform blocks as needed
    };

    m_ShaderStorageBlockBindings = {
        {"LightsData", LIGHTS_DATA_BINDING_POINT}
        // Add more SSBO blocks as needed
    };

    // Auto-initialize
    Initialize();
}

ShaderManager& ShaderManager::GetInstance()
{
    static ShaderManager instance;
    return instance;
}

void ShaderManager::Initialize()
{
    auto logger = Logger::GetLogger();
    logger->info("ShaderManager: Initializing with metadata='{}' and config='{}'.",
        m_MetadataPath.string(), m_ConfigPath.string());

    if (!LoadMetadata()) {
        logger->warn("ShaderManager: Could not load metadata; using defaults.");
    }
    if (!LoadConfig()) {
        logger->error("ShaderManager: Failed to load config; initialization will be incomplete.");
        return;
    }

    LoadShaders();  // Build the initial set of shader objects

    // If driver changed or profile changed, we rebuild everything
    if (IsGlobalMetadataChanged()) {
        logger->info("ShaderManager: Global metadata changed; recompiling all shaders.");
        ReloadAllShaders();

        // Update global metadata to reflect the current driver
        m_GlobalMetadata.driverVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
        m_GlobalMetadata.openGLProfile = "core";
        SaveMetadata();
    }
}

bool ShaderManager::LoadMetadata()
{
    auto logger = Logger::GetLogger();
    logger->info("ShaderManager: Loading metadata from '{}'.", m_MetadataPath.string());

    if (!std::filesystem::exists(m_MetadataPath)) {
        logger->warn("ShaderManager: Metadata file not found at '{}'.", m_MetadataPath.string());
        return false;
    }

    std::ifstream file(m_MetadataPath);
    if (!file) {
        logger->error("ShaderManager: Failed to open metadata file '{}'.", m_MetadataPath.string());
        return false;
    }

    try {
        nlohmann::json jsonData;
        file >> jsonData;

        m_GlobalMetadata.driverVersion = jsonData["global"]["driver_version"].get<std::string>();
        m_GlobalMetadata.openGLProfile = jsonData["global"]["openGL_profile"].get<std::string>();

        // Rebuild our map from the JSON data
        for (auto& [name, shaderData] : jsonData["shaders"].items()) {
            ShaderMetadata metadata;
            metadata.isComputeShader = shaderData.value("is_compute_shader", false);
            metadata.binaryPath = shaderData["binary_path"].get<std::string>();

            for (auto& [stageName, pathStr] : shaderData["shader_stages"].items()) {
                GLenum shaderType = GetShaderTypeFromString(stageName);
                metadata.shaderStages[shaderType] = pathStr.get<std::string>();
            }

            m_ShadersMetadata[name] = metadata;
        }
    }
    catch (const std::exception& e) {
        logger->error("ShaderManager: Error parsing metadata '{}': {}", m_MetadataPath.string(), e.what());
        return false;
    }

    logger->info("ShaderManager: Metadata loaded successfully.");
    return true;
}

bool ShaderManager::SaveMetadata() const
{
    auto logger = Logger::GetLogger();
    logger->info("ShaderManager: Saving metadata to '{}'.", m_MetadataPath.string());

    nlohmann::json jsonData;
    // Write out global
    jsonData["global"]["driver_version"] = m_GlobalMetadata.driverVersion;
    jsonData["global"]["openGL_profile"] = m_GlobalMetadata.openGLProfile;

    // Write out each shader
    for (auto& [name, metadata] : m_ShadersMetadata) {
        jsonData["shaders"][name]["is_compute_shader"] = metadata.isComputeShader;
        jsonData["shaders"][name]["binary_path"] = metadata.binaryPath.string();

        nlohmann::json stagesJson;
        for (auto& [shaderType, path] : metadata.shaderStages) {
            std::string stageName;
            switch (shaderType) {
            case GL_VERTEX_SHADER:          stageName = "vertex";        break;
            case GL_FRAGMENT_SHADER:        stageName = "fragment";      break;
            case GL_GEOMETRY_SHADER:        stageName = "geometry";      break;
            case GL_TESS_CONTROL_SHADER:    stageName = "tess_control";  break;
            case GL_TESS_EVALUATION_SHADER: stageName = "tess_evaluation"; break;
            case GL_COMPUTE_SHADER:         stageName = "compute";       break;
            default:                        stageName = "unknown";       break;
            }
            stagesJson[stageName] = path.string();
        }
        jsonData["shaders"][name]["shader_stages"] = stagesJson;
    }

    std::ofstream file(m_MetadataPath);
    if (!file) {
        logger->error("ShaderManager: Could not open '{}' for writing.", m_MetadataPath.string());
        return false;
    }

    try {
        file << jsonData.dump(4);
    }
    catch (const std::exception& e) {
        logger->error("ShaderManager: Error writing metadata '{}': {}", m_MetadataPath.string(), e.what());
        return false;
    }

    logger->info("ShaderManager: Metadata saved successfully.");
    return true;
}

bool ShaderManager::LoadConfig()
{
    auto logger = Logger::GetLogger();
    logger->info("ShaderManager: Loading config from '{}'.", m_ConfigPath.string());

    if (!std::filesystem::exists(m_ConfigPath)) {
        logger->error("ShaderManager: Config file not found at '{}'.", m_ConfigPath.string());
        return false;
    }

    std::ifstream file(m_ConfigPath);
    if (!file) {
        logger->error("ShaderManager: Failed to open config file '{}'.", m_ConfigPath.string());
        return false;
    }

    try {
        nlohmann::json jsonData;
        file >> jsonData;

        // Expect something like jsonData["shaders"] = { "name": {...}, ... }
        for (auto& [name, shaderData] : jsonData["shaders"].items()) {
            ShaderMetadata metadata;
            metadata.isComputeShader = shaderData.value("is_compute_shader", false);
            metadata.binaryPath = shaderData.value("binary_path", "");

            for (auto& [stageName, pathStr] : shaderData["shader_stages"].items()) {
                GLenum shaderType = GetShaderTypeFromString(stageName);
                metadata.shaderStages[shaderType] = pathStr.get<std::string>();
            }

            m_ShadersMetadata[name] = metadata;
        }
    }
    catch (const std::exception& e) {
        logger->error("ShaderManager: Error parsing config '{}': {}", m_ConfigPath.string(), e.what());
        return false;
    }

    logger->info("ShaderManager: Config loaded successfully.");
    return true;
}

void ShaderManager::LoadShaders()
{
    auto logger = Logger::GetLogger();
    logger->info("ShaderManager: Creating all shaders from config/metadata.");

    for (auto& [name, metadata] : m_ShadersMetadata) {
        try {
            std::shared_ptr<BaseShader> shader;

            // Create the appropriate type of shader
            if (metadata.isComputeShader) {
                // Expect exactly one stage: GL_COMPUTE_SHADER
                auto it = metadata.shaderStages.find(GL_COMPUTE_SHADER);
                if (it == metadata.shaderStages.end()) {
                    logger->error("ShaderManager: Compute shader '{}' has no compute stage defined.", name);
                    continue;
                }
                shader = std::make_shared<ComputeShader>(it->second, metadata.binaryPath);
                logger->info("ShaderManager: Loaded ComputeShader '{}'.", name);
            }
            else {
                // Multi-stage or standard vertex/fragment shader
                shader = std::make_shared<Shader>(metadata.shaderStages, metadata.binaryPath);
                logger->info("ShaderManager: Loaded Shader '{}'.", name);
            }

            m_Shaders[name] = shader;

            // Check if the binary is out-of-date
            if (IsShaderOutdated(name)) {
                shader->ReloadShader();
                logger->info("ShaderManager: Shader '{}' was outdated; reloaded from source.", name);
            }
            else if (!shader->LoadBinary()) {
                // If the binary can’t be loaded, recompile
                shader->ReloadShader();
                logger->info("ShaderManager: Shader '{}' could not load binary; recompiled.", name);
            }
            else {
                logger->debug("ShaderManager: Shader '{}' is up-to-date (loaded from binary).", name);
            }
        }
        catch (const std::exception& e) {
            logger->error("ShaderManager: Failed to load shader '{}': {}", name, e.what());
        }
    }
}

void ShaderManager::ReloadAllShaders()
{
    auto logger = Logger::GetLogger();
    logger->info("ShaderManager: Reloading all shaders.");

    for (auto& [name, shader] : m_Shaders) {
        try {
            shader->ReloadShader();
            logger->info("ShaderManager: Reloaded shader '{}'.", name);

            // Rebind uniform/storage blocks after reload
            RebindUniformBlocks(name);
            RebindShaderStorageBlocks(name);
        }
        catch (const std::exception& e) {
            logger->error("ShaderManager: Error reloading shader '{}': {}", name, e.what());
        }
    }
}

void ShaderManager::BindShader(std::string_view shaderName)
{
    auto logger = Logger::GetLogger();
    if (shaderName == m_CurrentlyBoundShader) {
        logger->debug("ShaderManager: '{}' is already bound; skipping re-bind.", shaderName);
        return;
    }

    auto it = m_Shaders.find(std::string(shaderName));
    if (it == m_Shaders.end()) {
        logger->error("ShaderManager: Shader '{}' not found; cannot bind.", shaderName);
        return;
    }

    auto shader = it->second;
    try {
        shader->Bind(); // glUseProgram(...)
        m_CurrentlyBoundShader = std::string(shaderName);
        logger->debug("ShaderManager: Bound shader '{}'.", shaderName);
    }
    catch (const std::exception& e) {
        logger->error("ShaderManager: Failed to bind shader '{}': {}", shaderName, e.what());
    }
}

std::shared_ptr<BaseShader> ShaderManager::GetCurrentlyBoundShader() const
{
    auto it = m_Shaders.find(m_CurrentlyBoundShader);
    if (it != m_Shaders.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<Shader> ShaderManager::GetShader(std::string_view name)
{
    auto it = m_Shaders.find(std::string(name));
    if (it == m_Shaders.end()) {
        Logger::GetLogger()->error("ShaderManager: Could not find standard shader '{}'.", name);
        return nullptr;
    }
    // Attempt dynamic cast
    return std::dynamic_pointer_cast<Shader>(it->second);
}

std::shared_ptr<ComputeShader> ShaderManager::GetComputeShader(std::string_view name)
{
    auto it = m_Shaders.find(std::string(name));
    if (it == m_Shaders.end()) {
        Logger::GetLogger()->error("ShaderManager: Could not find compute shader '{}'.", name);
        return nullptr;
    }
    return std::dynamic_pointer_cast<ComputeShader>(it->second);
}

void ShaderManager::RebindUniformBlocks(const std::string& shaderName)
{
    auto shaderPtr = GetShader(shaderName);
    if (!shaderPtr) {
        Logger::GetLogger()->error("ShaderManager: RebindUniformBlocks: Shader '{}' not found or invalid type.", shaderName);
        return;
    }

    for (auto& [blockName, bindingPoint] : m_UniformBlockBindings) {
        shaderPtr->BindUniformBlock(blockName, bindingPoint);
    }
}

void ShaderManager::RebindShaderStorageBlocks(const std::string& shaderName)
{
    auto shaderPtr = GetShader(shaderName);
    if (!shaderPtr) {
        Logger::GetLogger()->error("ShaderManager: RebindShaderStorageBlocks: Shader '{}' not found or invalid type.", shaderName);
        return;
    }

    for (auto& [blockName, bindingPoint] : m_ShaderStorageBlockBindings) {
        shaderPtr->BindShaderStorageBlock(blockName, bindingPoint);
    }
}

bool ShaderManager::IsGlobalMetadataChanged() const
{
    // Example logic: if current driver or profile mismatch stored
    std::string currentDriverVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    std::string currentProfile = "core";

    return (m_GlobalMetadata.driverVersion != currentDriverVersion ||
        m_GlobalMetadata.openGLProfile != currentProfile);
}

bool ShaderManager::IsShaderOutdated(const std::string& shaderName) const
{
    auto logger = Logger::GetLogger();
    auto it = m_ShadersMetadata.find(shaderName);
    if (it == m_ShadersMetadata.end()) {
        logger->error("ShaderManager: No metadata found for '{}'.", shaderName);
        return false; // can't check, assume not outdated
    }

    const auto& metadata = it->second;
    // If no binary path or file doesn't exist, we assume it’s outdated
    if (metadata.binaryPath.empty() || !std::filesystem::exists(metadata.binaryPath)) {
        return true;
    }

    // Compare last-mod times among all source files vs. the binary
    std::filesystem::file_time_type latestSourceTime = std::filesystem::file_time_type::min();
    std::unordered_set<std::string> processedFiles; // track #include recursion

    for (auto& [stageType, sourcePath] : metadata.shaderStages) {
        auto sourceTime = GetLatestShaderModificationTime(sourcePath, processedFiles);
        if (sourceTime > latestSourceTime) {
            latestSourceTime = sourceTime;
        }
    }

    auto binaryLastModified = std::filesystem::last_write_time(metadata.binaryPath);
    return (latestSourceTime > binaryLastModified);
}

std::filesystem::file_time_type ShaderManager::GetLatestShaderModificationTime(
    const std::filesystem::path& sourcePath,
    std::unordered_set<std::string>& processedFiles) const
{
    auto logger = Logger::GetLogger();
    auto canonicalPath = std::filesystem::weakly_canonical(sourcePath).string();

    // Avoid re-processing a file we've already visited
    if (processedFiles.count(canonicalPath)) {
        return std::filesystem::file_time_type::min();
    }
    processedFiles.insert(canonicalPath);

    if (!std::filesystem::exists(sourcePath)) {
        logger->error("ShaderManager: Source file '{}' does not exist.", sourcePath.string());
        throw std::runtime_error("ShaderManager: Missing shader file: " + sourcePath.string());
    }

    // Start with the direct file's last_write_time
    auto latestTime = std::filesystem::last_write_time(sourcePath);

    // Read the file to detect #includes
    std::ifstream file(sourcePath);
    if (!file) {
        logger->error("ShaderManager: Could not open file '{}'.", sourcePath.string());
        throw std::runtime_error("ShaderManager: Could not open file: " + sourcePath.string());
    }

    std::string line;
    bool inBlockComment = false;
    while (std::getline(file, line)) {
        std::string trimmed = line;

        // Handle block comment status
        if (inBlockComment) {
            if (auto endPos = trimmed.find("*/"); endPos != std::string::npos) {
                inBlockComment = false;
                trimmed = trimmed.substr(endPos + 2);
            }
            else {
                continue; // entire line is in block comment
            }
        }

        if (auto startPos = trimmed.find("/*"); startPos != std::string::npos) {
            inBlockComment = true;
            trimmed = trimmed.substr(0, startPos);
        }

        // Remove single-line comments
        if (auto commentPos = trimmed.find("//"); commentPos != std::string::npos) {
            trimmed.erase(commentPos);
        }

        // Trim whitespace
        while (!trimmed.empty() && (trimmed.back() == ' ' || trimmed.back() == '\t')) {
            trimmed.pop_back();
        }

        // Look for #include
        static const std::string includeDirective = "#include";
        if (trimmed.find(includeDirective) == 0) {
            // e.g.: #include "myInclude.glsl"
            auto startQuote = trimmed.find_first_of("\"<");
            auto endQuote = trimmed.find_first_of("\">", startQuote + 1);

            if (startQuote == std::string::npos || endQuote == std::string::npos) {
                logger->error("ShaderManager: Invalid #include syntax in '{}': {}", sourcePath.string(), line);
                throw std::runtime_error("ShaderManager: Invalid #include syntax in file: " + sourcePath.string());
            }

            std::string includeRelPath = trimmed.substr(startQuote + 1, endQuote - (startQuote + 1));
            auto includePath = sourcePath.parent_path() / includeRelPath;
            includePath = std::filesystem::weakly_canonical(includePath);

            if (!std::filesystem::exists(includePath)) {
                logger->warn("ShaderManager: Included file does not exist: {}", includePath.string());
                continue;
            }

            // Recursively check that file’s last write time
            auto includeTime = GetLatestShaderModificationTime(includePath, processedFiles);
            if (includeTime > latestTime) {
                latestTime = includeTime;
            }
        }
    }

    return latestTime;
}

GLenum ShaderManager::GetShaderTypeFromString(const std::string& type) const
{
    if (type == "vertex")          return GL_VERTEX_SHADER;
    if (type == "fragment")        return GL_FRAGMENT_SHADER;
    if (type == "geometry")        return GL_GEOMETRY_SHADER;
    if (type == "tess_control")    return GL_TESS_CONTROL_SHADER;
    if (type == "tess_evaluation") return GL_TESS_EVALUATION_SHADER;
    if (type == "compute")         return GL_COMPUTE_SHADER;

    throw std::runtime_error("ShaderManager: Unknown shader type string: " + type);
}