#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <string_view>
#include <filesystem>

#include "Graphics/Meshes/Model.h"
#include "Graphics/Buffers/MeshBuffer.h"
#include "Graphics/Meshes/MeshLayout.h"

class ModelManager {
public:
    static ModelManager& GetInstance();

    std::shared_ptr<Model> GetModel(std::string_view modelName);
    bool DeleteModel(std::string_view modelName);

    std::vector<std::shared_ptr<MeshBuffer>> GetModelMeshBuffers(std::string_view modelName, const MeshLayout& layout);
    const std::vector<MeshInfo>& GetModelMeshInfos(std::string_view modelName);

private:
    ModelManager();
    ~ModelManager() = default;

    ModelManager(const ModelManager&) = delete;
    ModelManager& operator=(const ModelManager&) = delete;

    std::unordered_map<std::string, std::shared_ptr<Model>> m_Models;
    std::unordered_map<std::string, std::filesystem::path> m_ModelPaths;
};