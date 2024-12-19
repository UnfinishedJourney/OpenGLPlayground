//#pragma once
//
//#include <string>
//#include <unordered_map>
//#include <memory>
//#include <string_view>
//#include <filesystem>
//#include <future>
//#include <shared_mutex>
//#include "Graphics/Meshes/Model.h"
//#include "Graphics/Buffers/MeshBuffer.h"
//#include "Graphics/Meshes/MeshLayout.h"
//
//class ModelManager {
//public:
//    static ModelManager& GetInstance();
//
//    std::future<std::shared_ptr<Model>> LoadModelAsync(std::string_view modelName, bool centerModel = true);
//    std::shared_ptr<Model> GetModel(std::string_view modelName);
//
//    bool DeleteModel(std::string_view modelName);
//
//    std::vector<std::shared_ptr<MeshBuffer>> GetModelMeshBuffers(std::string_view modelName, const MeshLayout& layout);
//    const std::vector<MeshInfo>& GetModelMeshInfos(std::string_view modelName);
//
//    void Clear();
//
//private:
//    ModelManager();
//    ~ModelManager() = default;
//
//    ModelManager(const ModelManager&) = delete;
//    ModelManager& operator=(const ModelManager&) = delete;
//
//    std::shared_ptr<Model> LoadModel(const std::string& modelName, const std::filesystem::path& modelPath, bool centerModel);
//
//    std::unordered_map<std::string, std::shared_ptr<Model>> m_Models;
//    std::unordered_map<std::string, std::filesystem::path> m_ModelPaths;
//
//    mutable std::shared_mutex m_CacheMutex;
//};