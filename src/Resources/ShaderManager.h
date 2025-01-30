#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <filesystem>

#include <glad/glad.h>
#include <nlohmann/json.hpp>

// Forward-declared to avoid including entire header
class BaseShader;
class ComputeShader;
class Shader;

/**
 * @struct GlobalMetadata
 * @brief Tracks basic system/driver info for potential caching or invalidation.
 */
struct GlobalMetadata
{
    std::string driverVersion;   ///< e.g., the OpenGL version string
    std::string openGLProfile;   ///< e.g., "core", "compatibility"
};

/**
 * @struct ShaderMetadata
 * @brief Holds info about a single (potentially multi-stage) shader,
 *        including source file paths, binary cache path, and flags.
 */
struct ShaderMetadata
{
    std::filesystem::path binaryPath;    ///< Where to load/save a program binary
    bool isComputeShader = false;        ///< If true, treat as a compute shader
    std::unordered_map<GLenum, std::filesystem::path> shaderStages; ///< Map of stage->file path
};

/**
 * @class ShaderManager
 * @brief Central manager for loading, reloading, and retrieving shader programs.
 *
 * Manages a set of shaders (vertex/fragment, geometry, compute, etc.), with support
 * for program binaries, auto-recompilation, uniform binding, and more.
 */
class ShaderManager
{
public:
    /**
     * @brief Constructs a ShaderManager, optionally specifying metadata/config paths.
     *
     * @param metadataPath File path to metadata JSON (tracks driver version, etc.).
     * @param configPath   File path to config JSON (lists available shaders).
     */
    ShaderManager(const std::filesystem::path& metadataPath = "../shaders/metadata.json",
        const std::filesystem::path& configPath = "../shaders/config.json");

    /**
     * @brief Destructor (default).
     */
    ~ShaderManager() = default;

    /**
     * @brief Retrieves the global singleton instance.
     */
    static ShaderManager& GetInstance();

    /**
     * @brief Called internally by the constructor to load config/metadata and then load shaders.
     */
    void Initialize();

    /**
     * @brief Retrieves a standard (non-compute) shader by name.
     *
     * @param name The name used in config/metadata for the shader.
     * @return A shared pointer to the Shader, or nullptr if not found or invalid type.
     */
    std::shared_ptr<Shader> GetShader(std::string_view name);

    /**
     * @brief Retrieves a compute shader by name.
     *
     * @param name The name used in config/metadata for the shader.
     * @return A shared pointer to the ComputeShader, or nullptr if not found or invalid type.
     */
    std::shared_ptr<ComputeShader> GetComputeShader(std::string_view name);

    /**
     * @brief Reloads all managed shaders (useful if sources changed on disk).
     */
    void ReloadAllShaders();

    /**
     * @brief Binds a named shader for subsequent draw calls.
     *
     * @param shaderName The name of the shader to bind.
     */
    void BindShader(std::string_view shaderName);

    /**
     * @brief Returns the currently bound shader (if any).
     */
    std::shared_ptr<BaseShader> GetCurrentlyBoundShader() const;

    /**
     * @brief Rebinds uniform blocks for a given shader name (if it exists).
     */
    void RebindUniformBlocks(const std::string& shaderName);

    /**
     * @brief Rebinds shader storage blocks for a given shader name (if it exists).
     */
    void RebindShaderStorageBlocks(const std::string& shaderName);

    /**
     * @brief Retrieves the map of all loaded shaders (by name).
     */
    const std::unordered_map<std::string, std::shared_ptr<BaseShader>>& GetShaders() const
    {
        return m_Shaders;
    }

private:
    // -----------------------------------------------------------------------
    // Private Methods
    // -----------------------------------------------------------------------
    bool LoadMetadata();
    bool SaveMetadata() const;
    bool LoadConfig();
    void LoadShaders();

    /**
     * @brief Checks if the stored driverVersion/profile differ from the current system.
     *        If so, triggers a full recompile.
     */
    bool IsGlobalMetadataChanged() const;

    /**
     * @brief Determines if a given shader's source code is newer than its binary file (if any).
     *
     * @param shaderName The key (name) of the shader in m_ShadersMetadata.
     * @return True if the shader source is newer (outdated binary), false if up-to-date.
     */
    bool IsShaderOutdated(const std::string& shaderName) const;

    /**
     * @brief Recursively find the latest modification time for a source file, including #includes.
     */
    std::filesystem::file_time_type GetLatestShaderModificationTime(
        const std::filesystem::path& sourcePath,
        std::unordered_set<std::string>& processedFiles) const;

    /**
     * @brief Maps a string like "vertex" or "compute" to the corresponding GLenum (GL_VERTEX_SHADER, etc.).
     */
    GLenum GetShaderTypeFromString(const std::string& type) const;

    // -----------------------------------------------------------------------
    // Member Variables
    // -----------------------------------------------------------------------
    std::filesystem::path m_MetadataPath; ///< JSON storing global metadata (driver version, etc.)
    std::filesystem::path m_ConfigPath;   ///< JSON describing available shaders/stages/paths

    std::string m_CurrentlyBoundShader;   ///< Track which shader is currently in use

    GlobalMetadata m_GlobalMetadata; ///< Driver/version info loaded from metadata
    std::unordered_map<std::string, ShaderMetadata> m_ShadersMetadata; ///< Info about each named shader
    std::unordered_map<std::string, std::shared_ptr<BaseShader>> m_Shaders; ///< Actual loaded shaders

    // Example: uniform/storage block binding maps
    std::unordered_map<std::string, GLuint> m_UniformBlockBindings;
    std::unordered_map<std::string, GLuint> m_ShaderStorageBlockBindings;

    static constexpr GLuint FRAME_DATA_BINDING_POINT = 0;
    static constexpr GLuint LIGHTS_DATA_BINDING_POINT = 1;
};