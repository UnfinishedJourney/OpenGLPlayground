#include "Resources/ModelManager.h"
#include "Utilities/Logger.h"

ModelManager& ModelManager::GetInstance() {
    static ModelManager instance;
    return instance;
}

ModelManager::ModelManager() {
    // Initialize model paths
    m_ModelPaths = {
        {"pig", "../assets/pig_triangulated.obj"},
        // Add other models as needed
    };
}

std::shared_ptr<Model> ModelManager::GetModel(std::string_view modelName) {
    auto it = m_Models.find(std::string(modelName));
    if (it != m_Models.end()) {
        return it->second;
    }

    // Load model if not found
    auto modelIt = m_ModelPaths.find(std::string(modelName));
    if (modelIt == m_ModelPaths.end()) {
        Logger::GetLogger()->error("Model '{}' not found.", modelName);
        return nullptr;
    }

    auto modelPath = modelIt->second;
    if (!std::filesystem::exists(modelPath)) {
        Logger::GetLogger()->warn("Model file '{}' does not exist.", modelPath.string());
        return nullptr;
    }

    auto model = std::make_shared<Model>(modelPath.string());
    m_Models[std::string(modelName)] = model;
    return model;
}

bool ModelManager::DeleteModel(std::string_view modelName) {
    return m_Models.erase(std::string(modelName)) > 0;
}

std::vector<std::shared_ptr<MeshBuffer>> ModelManager::GetModelMeshBuffers(std::string_view modelName, const MeshLayout& layout) {
    auto model = GetModel(modelName);
    if (!model) {
        Logger::GetLogger()->error("Model '{}' not found. Cannot create MeshBuffers.", modelName);
        return {};
    }

    return model->GetMeshBuffers(layout);
}

const std::vector<MeshInfo>& ModelManager::GetModelMeshInfos(std::string_view modelName) {
    auto model = GetModel(modelName);
    if (!model) {
        Logger::GetLogger()->error("Model '{}' not found. Cannot get MeshInfos.", modelName);
        static const std::vector<MeshInfo> empty;
        return empty;
    }

    return model->GetMeshesInfo();
}