#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <filesystem>
#include <unordered_set>
#include <nlohmann/json.hpp>

#include "Graphics/Shaders/BaseShader.h"
#include "Graphics/Shaders/Shader.h"
#include "Graphics/Shaders/ComputeShader.h"

struct GlobalMetadata {
    std::string driverVersion;
    std::string openGLProfile;
};

struct ShaderMetadata {
    std::filesystem::path binaryPath;
    bool isComputeShader = false;
    std::unordered_map<GLenum, std::filesystem::path> shaderStages;
};

class ShaderManager {
public:
    ShaderManager(const std::filesystem::path& metadataPath = "../shaders/metadata.json", const std::filesystem::path& configPath = "../shaders/config.json");
    ~ShaderManager() = default;

    static ShaderManager& GetInstance();
    void Initialize();

    std::shared_ptr<Shader> GetShader(std::string_view name);
    std::shared_ptr<ComputeShader> GetComputeShader(std::string_view name);
    void ReloadAllShaders();
    void BindShader(std::string_view shaderName);

    std::shared_ptr<BaseShader> GetCurrentlyBoundShader() const;

    // New methods for rebinding
    void RebindUniformBlocks(const std::string& shaderName);
    void RebindShaderStorageBlocks(const std::string& shaderName);

    // Access to all shaders
    const std::unordered_map<std::string, std::shared_ptr<BaseShader>>& GetShaders() const {
        return m_Shaders;
    }

private:
    std::filesystem::path m_MetadataPath;
    std::filesystem::path m_ConfigPath;
    std::string m_CurrentlyBoundShader;

    GlobalMetadata m_GlobalMetadata;
    std::unordered_map<std::string, ShaderMetadata> m_ShadersMetadata;
    std::unordered_map<std::string, std::shared_ptr<BaseShader>> m_Shaders;

    // Uniform Block and Shader Storage Block Binding Points
    std::unordered_map<std::string, GLuint> m_UniformBlockBindings;
    std::unordered_map<std::string, GLuint> m_ShaderStorageBlockBindings;

    // Binding Points Constants
    static constexpr GLuint FRAME_DATA_BINDING_POINT = 0;
    static constexpr GLuint LIGHTS_DATA_BINDING_POINT = 1;

    bool LoadMetadata();
    bool SaveMetadata() const;
    bool LoadConfig();
    void LoadShaders();

    bool IsGlobalMetadataChanged() const;
    bool IsShaderOutdated(const std::string& shaderName) const;

    GLenum GetShaderTypeFromString(const std::string& type) const;

    // Utility function to get the latest modification time of shader and its includes
    std::filesystem::file_time_type GetLatestShaderModificationTime(
        const std::filesystem::path& sourcePath,
        std::unordered_set<std::string>& processedFiles) const;
};