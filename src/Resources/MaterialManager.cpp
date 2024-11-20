#include "Resources/MaterialManager.h"
#include "Utilities/Logger.h"

MaterialManager& MaterialManager::GetInstance() {
    static MaterialManager instance;
    return instance;
}

std::shared_ptr<Material> MaterialManager::GetMaterial(std::string_view materialName) {
    auto it = m_Materials.find(std::string(materialName));
    if (it != m_Materials.end()) {
        return it->second;
    }
    else {
        Logger::GetLogger()->warn("Material '{}' not found.", materialName);
        return nullptr;
    }
}

void MaterialManager::AddMaterial(std::string_view name, const std::shared_ptr<Material>& material) {
    m_Materials.emplace(std::string(name), material);
}

void MaterialManager::BindMaterial(std::string_view name, const std::shared_ptr<BaseShader>& shader) {
    auto material = GetMaterial(name);
    if (!material) {
        Logger::GetLogger()->error("Material '{}' not found. Cannot bind.", name);
        return;
    }

    try {
        material->Bind(shader);
        m_CurrentlyBoundMaterial = std::string(name);
        Logger::GetLogger()->debug("Material '{}' bound successfully.", name);
    }
    catch (const std::exception& e) {
        Logger::GetLogger()->error("Failed to bind material '{}': {}", name, e.what());
    }
}

void MaterialManager::UnbindMaterial() {
    if (m_CurrentlyBoundMaterial.empty()) {
        return;
    }

    auto material = GetMaterial(m_CurrentlyBoundMaterial);
    if (material) {
        material->Unbind();
        Logger::GetLogger()->debug("Material '{}' unbound successfully.", m_CurrentlyBoundMaterial);
    }

    m_CurrentlyBoundMaterial.clear();
}