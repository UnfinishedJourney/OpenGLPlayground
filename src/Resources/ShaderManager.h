#pragma once

#include <string>
#include <unordered_map>
#include <filesystem>
#include <memory>
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
    std::filesystem::path sourcePath;
    std::filesystem::path binaryPath;
    bool isComputeShader = false;
};

class ShaderManager {
public:
    ShaderManager(const std::filesystem::path& metadataPath, const std::filesystem::path& configPath);
    ~ShaderManager() = default;

    void Initialize();

    std::shared_ptr<Shader> GetShader(std::string_view name);
    std::shared_ptr<ComputeShader> GetComputeShader(std::string_view name);
    void ReloadAllShaders();
    void BindShader(std::string_view shaderName);

    std::shared_ptr<BaseShader> GetCurrentlyBoundShader() const;

private:
    std::filesystem::path m_MetadataPath;
    std::filesystem::path m_ConfigPath;
    std::string m_CurrentlyBoundShader;

    GlobalMetadata m_GlobalMetadata;
    std::unordered_map<std::string, ShaderMetadata> m_ShadersMetadata;
    std::unordered_map<std::string, std::shared_ptr<BaseShader>> m_Shaders;

    bool LoadMetadata();
    bool SaveMetadata() const;
    bool LoadConfig();
    void LoadShaders();

    bool IsGlobalMetadataChanged() const;
    bool IsShaderOutdated(const std::string& shaderName) const;

    // Utility function to get the latest modification time of shader and its includes
    std::filesystem::file_time_type GetLatestShaderModificationTime(
        const std::filesystem::path& sourcePath,
        std::unordered_set<std::string>& processedFiles) const;
};