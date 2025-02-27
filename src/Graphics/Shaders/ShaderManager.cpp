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

namespace graphics {

    ShaderManager::ShaderManager(const std::filesystem::path& metadataPath,
        const std::filesystem::path& configPath)
        : metadataPath_(metadataPath), configPath_(configPath)
    {
        Initialize();
    }

    ShaderManager& ShaderManager::GetInstance() {
        static ShaderManager instance;
        return instance;
    }

    void ShaderManager::Initialize() {
        Logger::GetLogger()->info("ShaderManager: Initializing with metadata='{}' and config='{}'.",
            metadataPath_.string(), configPath_.string());
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
            globalMetadata_.driverVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
            globalMetadata_.openGLProfile = "core";
            SaveMetadata();
        }
    }

    bool ShaderManager::LoadMetadata() {
        Logger::GetLogger()->info("ShaderManager: Loading metadata from '{}'.", metadataPath_.string());
        if (!std::filesystem::exists(metadataPath_)) {
            Logger::GetLogger()->warn("ShaderManager: Metadata file not found at '{}'.", metadataPath_.string());
            return false;
        }
        std::ifstream file(metadataPath_);
        if (!file) {
            Logger::GetLogger()->error("ShaderManager: Failed to open metadata file '{}'.", metadataPath_.string());
            return false;
        }
        try {
            nlohmann::json jsonData;
            file >> jsonData;
            globalMetadata_.driverVersion = jsonData["global"]["driver_version"].get<std::string>();
            globalMetadata_.openGLProfile = jsonData["global"]["openGL_profile"].get<std::string>();
            for (auto& [name, shaderData] : jsonData["shaders"].items()) {
                ShaderMetadata metadata;
                metadata.isComputeShader = shaderData.value("is_compute_shader", false);
                metadata.binaryPath = shaderData["binary_path"].get<std::string>();
                for (auto& [stageName, pathStr] : shaderData["shader_stages"].items()) {
                    GLenum shaderType = shaderUtils::GetShaderTypeFromString(stageName);
                    metadata.shaderStages[shaderType] = pathStr.get<std::string>();
                }
                shadersMetadata_[name] = metadata;
            }
        }
        catch (const std::exception& e) {
            Logger::GetLogger()->error("ShaderManager: Error parsing metadata '{}': {}", metadataPath_.string(), e.what());
            return false;
        }
        Logger::GetLogger()->info("ShaderManager: Metadata loaded successfully.");
        return true;
    }

    bool ShaderManager::SaveMetadata() const {
        Logger::GetLogger()->info("ShaderManager: Saving metadata to '{}'.", metadataPath_.string());
        nlohmann::json jsonData;
        jsonData["global"]["driver_version"] = globalMetadata_.driverVersion;
        jsonData["global"]["openGL_profile"] = globalMetadata_.openGLProfile;
        for (auto& [name, metadata] : shadersMetadata_) {
            jsonData["shaders"][name]["is_compute_shader"] = metadata.isComputeShader;
            jsonData["shaders"][name]["binary_path"] = metadata.binaryPath.string();
            nlohmann::json stagesJson;
            for (auto& [shaderType, path] : metadata.shaderStages) {
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
        std::ofstream file(metadataPath_);
        if (!file) {
            Logger::GetLogger()->error("ShaderManager: Could not open '{}' for writing metadata.", metadataPath_.string());
            return false;
        }
        try {
            file << jsonData.dump(4);
        }
        catch (const std::exception& e) {
            Logger::GetLogger()->error("ShaderManager: Error writing metadata '{}': {}", metadataPath_.string(), e.what());
            return false;
        }
        Logger::GetLogger()->info("ShaderManager: Metadata saved successfully.");
        return true;
    }

    bool ShaderManager::LoadConfig() {
        Logger::GetLogger()->info("ShaderManager: Loading config from '{}'.", configPath_.string());
        if (!std::filesystem::exists(configPath_)) {
            Logger::GetLogger()->error("ShaderManager: Config file not found at '{}'.", configPath_.string());
            return false;
        }
        std::ifstream file(configPath_);
        if (!file) {
            Logger::GetLogger()->error("ShaderManager: Failed to open config file '{}'.", configPath_.string());
            return false;
        }
        try {
            nlohmann::json jsonData;
            file >> jsonData;
            for (auto& [name, shaderData] : jsonData["shaders"].items()) {
                ShaderMetadata metadata;
                metadata.isComputeShader = shaderData.value("is_compute_shader", false);
                metadata.binaryPath = shaderData.value("binary_path", "");
                for (auto& [stageName, pathStr] : shaderData["shader_stages"].items()) {
                    GLenum shaderType = shaderUtils::GetShaderTypeFromString(stageName);
                    metadata.shaderStages[shaderType] = pathStr.get<std::string>();
                }
                shadersMetadata_[name] = metadata;
            }
        }
        catch (const std::exception& e) {
            Logger::GetLogger()->error("ShaderManager: Error parsing config '{}': {}", configPath_.string(), e.what());
            return false;
        }
        Logger::GetLogger()->info("ShaderManager: Config loaded successfully.");
        return true;
    }

    void ShaderManager::LoadShaders() {
        Logger::GetLogger()->info("ShaderManager: Creating all shaders from config/metadata.");
        for (auto& [name, metadata] : shadersMetadata_) {
            try {
                std::shared_ptr<BaseShader> shader;
                if (metadata.isComputeShader) {
                    auto it = metadata.shaderStages.find(GL_COMPUTE_SHADER);
                    if (it == metadata.shaderStages.end()) {
                        Logger::GetLogger()->error("ShaderManager: Compute shader '{}' has no compute stage defined.", name);
                        continue;
                    }
                    shader = std::make_shared<ComputeShader>(it->second, metadata.binaryPath);
                    Logger::GetLogger()->info("ShaderManager: Loaded ComputeShader '{}'.", name);
                }
                else {
                    shader = std::make_shared<Shader>(metadata.shaderStages, metadata.binaryPath);
                    Logger::GetLogger()->info("ShaderManager: Loaded Shader '{}'.", name);
                }
                shaders_[name] = shader;

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
        for (auto& [name, shader] : shaders_) {
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
        if (std::string(shaderName) == currentlyBoundShader_) {
            Logger::GetLogger()->debug("ShaderManager: '{}' is already bound; skipping re-bind.", shaderName);
            return;
        }
        auto it = shaders_.find(std::string(shaderName));
        if (it == shaders_.end()) {
            Logger::GetLogger()->error("ShaderManager: Shader '{}' not found; cannot bind.", shaderName);
            return;
        }
        try {
            it->second->Bind();
            currentlyBoundShader_ = std::string(shaderName);
            Logger::GetLogger()->debug("ShaderManager: Bound shader '{}'.", shaderName);
        }
        catch (const std::exception& e) {
            Logger::GetLogger()->error("ShaderManager: Failed to bind shader '{}': {}", shaderName, e.what());
        }
    }

    std::shared_ptr<BaseShader> ShaderManager::GetCurrentlyBoundShader() const {
        auto it = shaders_.find(currentlyBoundShader_);
        if (it != shaders_.end())
            return it->second;
        return nullptr;
    }

    std::shared_ptr<Shader> ShaderManager::GetShader(std::string_view name) {
        auto it = shaders_.find(std::string(name));
        if (it == shaders_.end()) {
            Logger::GetLogger()->error("ShaderManager: Could not find standard shader '{}'.", name);
            return nullptr;
        }
        return std::dynamic_pointer_cast<Shader>(it->second);
    }

    std::shared_ptr<ComputeShader> ShaderManager::GetComputeShader(std::string_view name) {
        auto it = shaders_.find(std::string(name));
        if (it == shaders_.end()) {
            Logger::GetLogger()->error("ShaderManager: Could not find compute shader '{}'.", name);
            return nullptr;
        }
        return std::dynamic_pointer_cast<ComputeShader>(it->second);
    }

    const std::unordered_map<std::string, std::shared_ptr<BaseShader>>& ShaderManager::GetShaders() const {
        return shaders_;
    }

    bool ShaderManager::IsGlobalMetadataChanged() const {
        std::string currentDriverVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
        std::string currentProfile = "core";
        return (globalMetadata_.driverVersion != currentDriverVersion ||
            globalMetadata_.openGLProfile != currentProfile);
    }

    bool ShaderManager::IsShaderOutdated(const std::string& shaderName) const {
        auto logger = Logger::GetLogger();
        auto it = shadersMetadata_.find(shaderName);
        if (it == shadersMetadata_.end()) {
            logger->error("ShaderManager: No metadata found for '{}'.", shaderName);
            return false;
        }
        const auto& metadata = it->second;
        if (metadata.binaryPath.empty() || !std::filesystem::exists(metadata.binaryPath))
            return true;
        std::filesystem::file_time_type latestSourceTime = std::filesystem::file_time_type::min();
        std::unordered_set<std::string> processedFiles;
        for (auto& [stageType, sourcePath] : metadata.shaderStages) {
            auto sourceTime = shaderUtils::GetLatestModificationTime(sourcePath, processedFiles);
            if (sourceTime > latestSourceTime)
                latestSourceTime = sourceTime;
        }
        auto binaryLastModified = std::filesystem::last_write_time(metadata.binaryPath);
        return (latestSourceTime > binaryLastModified);
    }

    std::filesystem::file_time_type ShaderManager::GetLatestShaderModificationTime(
        const std::filesystem::path& sourcePath,
        std::unordered_set<std::string>& processedFiles) const {
        return shaderUtils::GetLatestModificationTime(sourcePath, processedFiles);
    }

} // namespace graphics