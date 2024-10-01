#pragma once

#include <nlohmann/json.hpp>
#include "Graphics/Shaders/Shader.h"
#include "Graphics/Shaders/ComputeShader.h"

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

class ShaderManager {
public:
    ShaderManager(const std::string& metadataPath, const std::string& configPath);
    ~ShaderManager() = default;

    void Initialize();

    std::shared_ptr<Shader> GetShader(const std::string& name);
    std::shared_ptr<ComputeShader> GetComputeShader(const std::string& name);

private:
    std::string m_MetadataPath;
    std::string m_ConfigPath;

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