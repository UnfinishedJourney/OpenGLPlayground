#include "ShaderManager.h"
#include "Utilities/Logger.h"
#include "BaseShader.h"
#include "Shader.h"
#include "ComputeShader.h"
#include <fstream>
#include <chrono>
#include <stdexcept>
#include <algorithm>
#include <nlohmann/json.hpp>
#include <filesystem>
#include "ShaderUtils.h"

namespace Graphics {

    ShaderManager::ShaderManager(const std::filesystem::path& metadataPath,
        const std::filesystem::path& configPath)
        : m_MetadataPath(metadataPath),
        m_ConfigPath(configPath)
    {

        Initialize();
    }

    ShaderManager& ShaderManager::GetInstance() { 
        static ShaderManager instance;
        return instance;
    }

    void ShaderManager::Initialize() {
        Logger::GetLogger()->info("ShaderManager: Initializing with metadata='{}' and config='{}'.",
            m_MetadataPath.string(), m_ConfigPath.string());
        if (!LoadMetadata()) {
            Logger::GetLogger()->warn("ShaderManager: Could not load metadata; using defaults.");
        }
        if (!LoadConfig()) {
            Logger::GetLogger()->error("ShaderManager: Failed to load config; initialization incomplete.");
            return;
        }
        LoadShaders();

        if (IsGlobalMetadataChanged()) {
            Logger::GetLogger()->info("ShaderManager: Global metadata changed; recompiling all shaders.");
            ReloadAllShaders();
            m_GlobalMetadata.DriverVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
            m_GlobalMetadata.OpenGLProfile = "core";
            SaveMetadata();
        }
    }

    bool ShaderManager::LoadMetadata() {
        Logger::GetLogger()->info("ShaderManager: Loading metadata from '{}'.", m_MetadataPath.string());
        if (!std::filesystem::exists(m_MetadataPath)) {
            Logger::GetLogger()->warn("ShaderManager: Metadata file not found at '{}'.", m_MetadataPath.string());
            return false;
        }
        std::ifstream file(m_MetadataPath);
        if (!file) {
            Logger::GetLogger()->error("ShaderManager: Failed to open metadata file '{}'.", m_MetadataPath.string());
            return false;
        }
        try {
            nlohmann::json jsonData;
            file >> jsonData;
            m_GlobalMetadata.DriverVersion = jsonData["global"]["driver_version"].get<std::string>();
            m_GlobalMetadata.OpenGLProfile = jsonData["global"]["openGL_profile"].get<std::string>();
            for (auto& [name, shaderData] : jsonData["shaders"].items()) {
                ShaderMetadata metadata;
                metadata.IsComputeShader = shaderData.value("is_compute_shader", false);
                metadata.BinaryPath = shaderData["binary_path"].get<std::string>();
                for (auto& [stageName, pathStr] : shaderData["shader_stages"].items()) {
                    GLenum shaderType = ShaderUtils::GetShaderTypeFromString(stageName);
                    metadata.ShaderStages[shaderType] = pathStr.get<std::string>();
                }
                m_ShadersMetadata[name] = metadata;
            }
        }
        catch (const std::exception& e) {
            Logger::GetLogger()->error("ShaderManager: Error parsing metadata '{}': {}", m_MetadataPath.string(), e.what());
            return false;
        }
        Logger::GetLogger()->info("ShaderManager: Metadata loaded successfully.");
        return true;
    }

    bool ShaderManager::SaveMetadata() const {
        Logger::GetLogger()->info("ShaderManager: Saving metadata to '{}'.", m_MetadataPath.string());
        nlohmann::json jsonData;
        jsonData["global"]["driver_version"] = m_GlobalMetadata.DriverVersion;
        jsonData["global"]["openGL_profile"] = m_GlobalMetadata.OpenGLProfile;
        for (auto& [name, metadata] : m_ShadersMetadata) {
            jsonData["shaders"][name]["is_compute_shader"] = metadata.IsComputeShader;
            jsonData["shaders"][name]["binary_path"] = metadata.BinaryPath.string();
            nlohmann::json stagesJson;
            for (auto& [shaderType, path] : metadata.ShaderStages) {
                std::string stageName;
                switch (shaderType) {
                case GL_VERTEX_SHADER:          stageName = "vertex"; break;
                case GL_FRAGMENT_SHADER:        stageName = "fragment"; break;
                case GL_GEOMETRY_SHADER:        stageName = "geometry"; break;
                case GL_TESS_CONTROL_SHADER:    stageName = "tess_control"; break;
                case GL_TESS_EVALUATION_SHADER: stageName = "tess_evaluation"; break;
                case GL_COMPUTE_SHADER:         stageName = "compute"; break;
                default:                        stageName = "unknown"; break;
                }
                stagesJson[stageName] = path.string();
            }
            jsonData["shaders"][name]["shader_stages"] = stagesJson;
        }
        std::ofstream file(m_MetadataPath);
        if (!file) {
            Logger::GetLogger()->error("ShaderManager: Could not open '{}' for writing metadata.", m_MetadataPath.string());
            return false;
        }
        try {
            file << jsonData.dump(4);
        }
        catch (const std::exception& e) {
            Logger::GetLogger()->error("ShaderManager: Error writing metadata '{}': {}", m_MetadataPath.string(), e.what());
            return false;
        }
        Logger::GetLogger()->info("ShaderManager: Metadata saved successfully.");
        return true;
    }

    bool ShaderManager::LoadConfig() {
        Logger::GetLogger()->info("ShaderManager: Loading config from '{}'.", m_ConfigPath.string());
        if (!std::filesystem::exists(m_ConfigPath)) {
            Logger::GetLogger()->error("ShaderManager: Config file not found at '{}'.", m_ConfigPath.string());
            return false;
        }
        std::ifstream file(m_ConfigPath);
        if (!file) {
            Logger::GetLogger()->error("ShaderManager: Failed to open config file '{}'.", m_ConfigPath.string());
            return false;
        }
        try {
            nlohmann::json jsonData;
            file >> jsonData;
            for (auto& [name, shaderData] : jsonData["shaders"].items()) {
                ShaderMetadata metadata;
                metadata.IsComputeShader = shaderData.value("is_compute_shader", false);
                metadata.BinaryPath = shaderData.value("binary_path", "");
                for (auto& [stageName, pathStr] : shaderData["shader_stages"].items()) {
                    GLenum shaderType = ShaderUtils::GetShaderTypeFromString(stageName);
                    metadata.ShaderStages[shaderType] = pathStr.get<std::string>();
                }
                m_ShadersMetadata[name] = metadata;
            }
        }
        catch (const std::exception& e) {
            Logger::GetLogger()->error("ShaderManager: Error parsing config '{}': {}", m_ConfigPath.string(), e.what());
            return false;
        }
        Logger::GetLogger()->info("ShaderManager: Config loaded successfully.");
        return true;
    }

    void ShaderManager::LoadShaders() {
        Logger::GetLogger()->info("ShaderManager: Creating all shaders from config/metadata.");
        for (auto& [name, metadata] : m_ShadersMetadata) {
            try {
                std::shared_ptr<BaseShader> shader;
                if (metadata.IsComputeShader) {
                    auto it = metadata.ShaderStages.find(GL_COMPUTE_SHADER);
                    if (it == metadata.ShaderStages.end()) {
                        Logger::GetLogger()->error("ShaderManager: Compute shader '{}' has no compute stage defined.", name);
                        continue;
                    }
                    shader = std::make_shared<ComputeShader>(it->second, metadata.BinaryPath);
                    Logger::GetLogger()->info("ShaderManager: Loaded ComputeShader '{}'.", name);
                }
                else {
                    shader = std::make_shared<Shader>(metadata.ShaderStages, metadata.BinaryPath);
                    Logger::GetLogger()->info("ShaderManager: Loaded Shader '{}'.", name);
                }
                m_Shaders[name] = shader;

                if (IsShaderOutdated(name)) {
                    shader->ReloadShader();
                    Logger::GetLogger()->info("ShaderManager: Shader '{}' was outdated; reloaded from source.", name);
                }
                else if (!shader->LoadBinary()) {
                    shader->ReloadShader();
                    Logger::GetLogger()->info("ShaderManager: Shader '{}' could not load binary; recompiled.", name);
                }
                else {
                    Logger::GetLogger()->debug("ShaderManager: Shader '{}' is up-to-date (loaded from binary).", name);
                }
            }
            catch (const std::exception& e) {
                Logger::GetLogger()->error("ShaderManager: Failed to load shader '{}': {}", name, e.what());
            }
        }
    }

    void ShaderManager::ReloadAllShaders() {
        Logger::GetLogger()->info("ShaderManager: Reloading all shaders.");
        for (auto& [name, shader] : m_Shaders) {
            try {
                shader->ReloadShader();
                Logger::GetLogger()->info("ShaderManager: Reloaded shader '{}'.", name);
            }
            catch (const std::exception& e) {
                Logger::GetLogger()->error("ShaderManager: Error reloading shader '{}': {}", name, e.what());
            }
        }
    }

    void ShaderManager::BindShader(std::string_view shaderName) {
        if (std::string(shaderName) == m_CurrentlyBoundShader) {
            Logger::GetLogger()->debug("ShaderManager: '{}' is already bound; skipping re-bind.", shaderName);
            return;
        }
        auto it = m_Shaders.find(std::string(shaderName));
        if (it == m_Shaders.end()) {
            Logger::GetLogger()->error("ShaderManager: Shader '{}' not found; cannot bind.", shaderName);
            return;
        }
        try {
            it->second->Bind();
            m_CurrentlyBoundShader = std::string(shaderName);
            Logger::GetLogger()->debug("ShaderManager: Bound shader '{}'.", shaderName);
        }
        catch (const std::exception& e) {
            Logger::GetLogger()->error("ShaderManager: Failed to bind shader '{}': {}", shaderName, e.what());
        }
    }

    std::shared_ptr<BaseShader> ShaderManager::GetCurrentlyBoundShader() const {
        auto it = m_Shaders.find(m_CurrentlyBoundShader);
        if (it != m_Shaders.end())
            return it->second;
        return nullptr;
    }

    std::shared_ptr<Shader> ShaderManager::GetShader(std::string_view name) {
        auto it = m_Shaders.find(std::string(name));
        if (it == m_Shaders.end()) {
            Logger::GetLogger()->error("ShaderManager: Could not find standard shader '{}'.", name);
            return nullptr;
        }
        return std::dynamic_pointer_cast<Shader>(it->second);
    }

    std::shared_ptr<ComputeShader> ShaderManager::GetComputeShader(std::string_view name) {
        auto it = m_Shaders.find(std::string(name));
        if (it == m_Shaders.end()) {
            Logger::GetLogger()->error("ShaderManager: Could not find compute shader '{}'.", name);
            return nullptr;
        }
        return std::dynamic_pointer_cast<ComputeShader>(it->second);
    }

    const std::unordered_map<std::string, std::shared_ptr<BaseShader>>& ShaderManager::GetShaders() const {
        return m_Shaders;
    }

    bool ShaderManager::IsGlobalMetadataChanged() const {
        std::string currentDriverVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
        std::string currentProfile = "core";  // For example.
        return (m_GlobalMetadata.DriverVersion != currentDriverVersion ||
            m_GlobalMetadata.OpenGLProfile != currentProfile);
    }

    bool ShaderManager::IsShaderOutdated(const std::string& shaderName) const {
        auto logger = Logger::GetLogger();
        auto it = m_ShadersMetadata.find(shaderName);
        if (it == m_ShadersMetadata.end()) {
            logger->error("ShaderManager: No metadata found for '{}'.", shaderName);
            return false; // Assume up-to-date if unknown.
        }
        const auto& metadata = it->second;
        if (metadata.BinaryPath.empty() || !std::filesystem::exists(metadata.BinaryPath))
            return true;
        std::filesystem::file_time_type latestSourceTime = std::filesystem::file_time_type::min();
        std::unordered_set<std::string> processedFiles;
        for (auto& [stageType, sourcePath] : metadata.ShaderStages) {
            auto sourceTime = ShaderUtils::GetLatestModificationTime(sourcePath, processedFiles);
            if (sourceTime > latestSourceTime)
                latestSourceTime = sourceTime;
        }
        auto binaryLastModified = std::filesystem::last_write_time(metadata.BinaryPath);
        return (latestSourceTime > binaryLastModified);
    }

    std::filesystem::file_time_type ShaderManager::GetLatestShaderModificationTime(
        const std::filesystem::path& SourcePath,
        std::unordered_set<std::string>& ProcessedFiles) const
    {
        // Forward to our ShaderUtils helper.
        return ShaderUtils::GetLatestModificationTime(SourcePath, ProcessedFiles);
    }

} // namespace Graphics