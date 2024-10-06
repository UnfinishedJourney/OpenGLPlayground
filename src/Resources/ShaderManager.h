#pragma once

#include <nlohmann/json.hpp>

#include <string>
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

struct GlobalMetadata {
    std::string driverVersion;
    std::string openGLProfile;
};

struct ShaderMetadata {
    std::string sourcePath;
    std::string binaryPath;
    std::filesystem::file_time_type sourceLastModified;
    std::filesystem::file_time_type binaryLastModified; //add more advanced checkup to recompile code
    bool isComputeShader;
};

//add include checkup
//add hot reloading
//use asynch loading

class BaseShader;
class ComputeShader;
class Shader;

class ShaderManager {
public:
    ShaderManager(const std::string& metadataPath, const std::string& configPath);
    ~ShaderManager() = default;

    void Initialize();

    std::shared_ptr<Shader> GetShader(const std::string& name);
    std::shared_ptr<BaseShader> GetCurrentlyBoundShader()
    {
        return m_Shaders[m_CurrentlyBoundShader];
    }
    std::shared_ptr<ComputeShader> GetComputeShader(const std::string& name);
    void ReloadAllShaders();
    void BindShader(const std::string& shaderName);

private:
    std::string m_MetadataPath;
    std::string m_ConfigPath;
    std::string m_CurrentlyBoundShader;

    bool m_MetaHasChanged;
    GlobalMetadata m_GlobalMetadata;
    std::unordered_map<std::string, ShaderMetadata> m_ShadersMetadata;
    std::unordered_map<std::string, std::shared_ptr<BaseShader>> m_Shaders;

    bool LoadMetadata();
    bool SaveMetadata();
    bool LoadConfig();
    void LoadShaders();

    bool IsGlobalMetadataChanged();
    bool IsShaderOutdated(const std::string& shaderName);
};