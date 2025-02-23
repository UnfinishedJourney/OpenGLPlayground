#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <filesystem>
#include <glad/glad.h>
#include <nlohmann/json.hpp>
#include "BaseShader.h"
#include "ComputeShader.h"
#include "Shader.h"

namespace graphics {

    /**
     * @brief Global metadata for shader caching.
     */
    struct GlobalMetadata {
        std::string driverVersion;
        std::string openGLProfile;
    };

    /**
     * @brief Metadata for an individual shader.
     */
    struct ShaderMetadata {
        std::filesystem::path binaryPath;
        bool isComputeShader = false;
        std::unordered_map<GLenum, std::filesystem::path> shaderStages;
    };

    /**
     * @brief Central manager for shader programs.
     */
    class ShaderManager {
    public:
        ~ShaderManager() = default;
        ShaderManager(const ShaderManager&) = delete;
        ShaderManager& operator=(const ShaderManager&) = delete;

        static ShaderManager& GetInstance();
        void Initialize();
        std::shared_ptr<Shader> GetShader(std::string_view name);
        std::shared_ptr<ComputeShader> GetComputeShader(std::string_view name);
        void ReloadAllShaders();
        void BindShader(std::string_view shaderName);
        std::shared_ptr<BaseShader> GetCurrentlyBoundShader() const;
        const std::unordered_map<std::string, std::shared_ptr<BaseShader>>& GetShaders() const;

    private:
        ShaderManager(const std::filesystem::path& metadataPath = "../configs/metadata_shaders.json",
            const std::filesystem::path& configPath = "../configs/shaders_config.json");
        bool LoadMetadata();
        bool SaveMetadata() const;
        bool LoadConfig();
        void LoadShaders();
        bool IsGlobalMetadataChanged() const;
        bool IsShaderOutdated(const std::string& shaderName) const;
        std::filesystem::file_time_type GetLatestShaderModificationTime(
            const std::filesystem::path& sourcePath,
            std::unordered_set<std::string>& processedFiles) const;

        std::filesystem::path metadataPath_;
        std::filesystem::path configPath_;
        std::string currentlyBoundShader_;

        GlobalMetadata globalMetadata_;
        std::unordered_map<std::string, ShaderMetadata> shadersMetadata_;
        std::unordered_map<std::string, std::shared_ptr<BaseShader>> shaders_;
    };

} // namespace graphics