#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <glad/glad.h>

// Forward declarations.
namespace graphics {
    class BaseShader;
    class ComputeShader;
    class Shader;
}

namespace graphics {

    /**
     * @struct GlobalMetadata
     * @brief Tracks basic system/driver info for caching or invalidation.
     */
    struct GlobalMetadata {
        std::string DriverVersion;  // e.g., OpenGL version string.
        std::string OpenGLProfile;  // e.g., "core", "compatibility".
    };

    /**
     * @struct ShaderMetadata
     * @brief Holds info about a shader, including its source file paths,
     *        binary cache path, and type flag.
     */
    struct ShaderMetadata {
        std::filesystem::path BinaryPath;  // Path to the binary.
        bool IsComputeShader = false;      // True if it’s a compute shader.
        std::unordered_map<GLenum, std::filesystem::path> ShaderStages;
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
        ShaderManager(const std::filesystem::path& MetadataPath = "../configs/metadata_shaders.json",
            const std::filesystem::path& ConfigPath = "../configs/shaders_config.json");
        bool LoadMetadata();
        bool SaveMetadata() const;
        bool LoadConfig();
        void LoadShaders();
        bool IsGlobalMetadataChanged() const;
        bool IsShaderOutdated(const std::string& shaderName) const;
        std::filesystem::file_time_type GetLatestShaderModificationTime(
            const std::filesystem::path& SourcePath,
            std::unordered_set<std::string>& ProcessedFiles) const;

        // --------------------
        // Member Variables
        // --------------------
        std::filesystem::path m_MetadataPath;
        std::filesystem::path m_ConfigPath;
        std::string m_CurrentlyBoundShader;

        GlobalMetadata m_GlobalMetadata;
        std::unordered_map<std::string, ShaderMetadata> m_ShadersMetadata;
        std::unordered_map<std::string, std::shared_ptr<BaseShader>> m_Shaders;

    };

} // namespace graphics