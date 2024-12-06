#include "Resources/ModelManager.h"
#include "Utilities/Logger.h"
#include <stdexcept>
#include <algorithm>

ModelManager& ModelManager::GetInstance() {
    static ModelManager instance;
    return instance;
}

ModelManager::ModelManager() {
    // Initialize known model paths
    m_ModelPaths = {
        {"pig", "../assets/Objs/pig_triangulated.obj"},
        {"bunny", "../assets/Objs/bunny.obj"},
        {"dragon", "../assets/Objs/dragon.obj"},
        {"bistro", "../assets/AmazonBistro/Exterior/exterior.obj"}
    };
}

std::future<std::shared_ptr<Model>> ModelManager::LoadModelAsync(std::string_view modelName, bool centerModel) {
    std::string modelNameStr(modelName);

    {
        std::shared_lock lock(m_CacheMutex);
        auto it = m_Models.find(modelNameStr);
        if (it != m_Models.end()) {
            Logger::GetLogger()->info("Model '{}' retrieved from cache (async).", modelNameStr);
            return std::async(std::launch::deferred, [model = it->second]() {
                return model;
                });
        }
    }

    auto pathIt = m_ModelPaths.find(modelNameStr);
    if (pathIt == m_ModelPaths.end()) {
        Logger::GetLogger()->error("Model '{}' path not found in ModelManager.", modelNameStr);
        return std::async(std::launch::deferred, []() -> std::shared_ptr<Model> { return nullptr; });
    }

    auto modelPath = pathIt->second;
    if (!std::filesystem::exists(modelPath)) {
        Logger::GetLogger()->warn("Model file '{}' does not exist.", modelPath.string());
        return std::async(std::launch::deferred, []() -> std::shared_ptr<Model> { return nullptr; });
    }

    return std::async(std::launch::async, [this, modelNameStr, modelPath, centerModel]() -> std::shared_ptr<Model> {
        try {
            auto model = LoadModel(modelNameStr, modelPath, centerModel);
            {
                std::unique_lock lock(m_CacheMutex);
                m_Models[modelNameStr] = model;
            }
            Logger::GetLogger()->info("Model '{}' loaded asynchronously.", modelNameStr);
            return model;
        }
        catch (const std::exception& e) {
            Logger::GetLogger()->error("Failed to load model '{}': {}", modelNameStr, e.what());
            return nullptr;
        }
        });
}

std::shared_ptr<Model> ModelManager::GetModel(std::string_view modelName) {
    std::string modelNameStr(modelName);

    {
        std::shared_lock lock(m_CacheMutex);
        auto it = m_Models.find(modelNameStr);
        if (it != m_Models.end()) {
            return it->second;
        }
    }

    auto pathIt = m_ModelPaths.find(modelNameStr);
    if (pathIt == m_ModelPaths.end()) {
        Logger::GetLogger()->error("Model '{}' path not found in ModelManager.", modelNameStr);
        return nullptr;
    }

    auto modelPath = pathIt->second;
    if (!std::filesystem::exists(modelPath)) {
        Logger::GetLogger()->warn("Model file '{}' does not exist.", modelPath.string());
        return nullptr;
    }

    try {
        auto model = LoadModel(modelNameStr, modelPath, true);
        {
            std::unique_lock lock(m_CacheMutex);
            m_Models[modelNameStr] = model;
        }
        Logger::GetLogger()->info("Model '{}' loaded synchronously.", modelNameStr);
        return model;
    }
    catch (const std::exception& e) {
        Logger::GetLogger()->error("Failed to load model '{}': {}", modelNameStr, e.what());
        return nullptr;
    }
}

std::shared_ptr<Model> ModelManager::LoadModel(const std::string& modelName, const std::filesystem::path& modelPath, bool centerModel) {
    return std::make_shared<Model>(modelPath.string(), centerModel);
}

bool ModelManager::DeleteModel(std::string_view modelName) {
    std::string modelNameStr(modelName);
    std::unique_lock lock(m_CacheMutex);
    size_t erased = m_Models.erase(modelNameStr);
    if (erased > 0) {
        Logger::GetLogger()->info("Model '{}' deleted from cache.", modelNameStr);
        return true;
    }
    Logger::GetLogger()->warn("Attempted to delete non-existent model '{}'.", modelNameStr);
    return false;
}

std::vector<std::shared_ptr<MeshBuffer>> ModelManager::GetModelMeshBuffers(std::string_view modelName, const MeshLayout& layout) {
    auto model = GetModel(modelName);
    if (!model) {
        Logger::GetLogger()->error("Model '{}' not found. Cannot retrieve MeshBuffers.", modelName);
        return {};
    }
    return model->GetMeshBuffers(layout);
}

const std::vector<MeshInfo>& ModelManager::GetModelMeshInfos(std::string_view modelName) {
    auto model = GetModel(modelName);
    if (!model) {
        Logger::GetLogger()->error("Model '{}' not found. Returning empty MeshInfo list.", modelName);
        static const std::vector<MeshInfo> empty;
        return empty;
    }
    return model->GetMeshesInfo();
}

void ModelManager::Clear() {
    std::unique_lock lock(m_CacheMutex);
    m_Models.clear();
    Logger::GetLogger()->info("All models cleared from ModelManager cache.");
}