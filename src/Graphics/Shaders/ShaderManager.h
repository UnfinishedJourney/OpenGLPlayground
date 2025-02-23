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
     * @struct GlobalMetadata
     * @brief Tracks basic system/driver info for caching or invalidation.
     */
    struct GlobalMetadata {
        std::string driverVersion;  ///< e.g., OpenGL version string.
        std::string openGLProfile;  ///< e.g., "core", "compatibility".
    };

    /**
     * @struct ShaderMetadata
     * @brief Holds info about a shader, including its source file paths,
     *        binary cache path, and type flag.
     */
    struct ShaderMetadata {
        std::filesystem::path binaryPath;  ///< Path to the binary.
        bool isComputeShader = false;      ///< True if it’s a compute shader.
        std::unordered_map<GLenum, std::filesystem::path> shaderStages;
    };

    /**
     * @class ShaderManager
     * @brief Central manager for loading, reloading, and retrieving shader programs.
     *
     * Manages a set of shaders with support for program binaries, auto-recompilation,
     * uniform block rebinding, and more.
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

        // Member Variables
        std::filesystem::path m_MetadataPath_;
        std::filesystem::path m_ConfigPath_;
        std::string m_CurrentlyBoundShader_;

        GlobalMetadata m_GlobalMetadata_;
        std::unordered_map<std::string, ShaderMetadata> m_ShadersMetadata_;
        std::unordered_map<std::string, std::shared_ptr<BaseShader>> m_Shaders_;
    };

} // namespace graphics