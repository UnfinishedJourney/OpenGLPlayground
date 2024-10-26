#pragma once

#include <string>
#include <unordered_map>
#include <filesystem>
#include <memory>
#include <shared_mutex>
#include <mutex>
#include <queue>
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
    std::filesystem::file_time_type sourceLastModified;
    std::filesystem::file_time_type binaryLastModified;
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

    mutable std::shared_mutex m_ShaderMutex;

    GlobalMetadata m_GlobalMetadata;
    std::unordered_map<std::string, ShaderMetadata> m_ShadersMetadata;
    std::unordered_map<std::string, std::shared_ptr<BaseShader>> m_Shaders;

    bool LoadMetadata();
    bool SaveMetadata() const;
    bool LoadConfig();
    void LoadShaders();

    bool IsGlobalMetadataChanged() const;
    bool IsShaderOutdated(const std::string& shaderName) const;

    // Command queue for OpenGL calls
    //void EnqueueGLCommand(std::function<void()> command);
    //void ExecuteGLCommands();

    //std::queue<std::function<void()>> m_GLCommandQueue;
    //mutable std::mutex m_GLCommandQueueMutex;
};