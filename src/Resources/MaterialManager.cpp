#include "MaterialManager.h"
#include "Utilities/Logger.h"

MaterialManager& MaterialManager::GetInstance() {
    static MaterialManager instance;
    return instance;
}

std::shared_ptr<Material> MaterialManager::GetMaterial(const std::string& name) {
    auto it = m_Materials.find(name);
    if (it != m_Materials.end()) {
        return it->second;
    }
    Logger::GetLogger()->warn("Material '{}' not found.", name);
    return nullptr;
}

void MaterialManager::AddMaterial(const std::string& name, const std::shared_ptr<Material>& material) {
    m_Materials[name] = material;
    material->SetName(name);
}

void MaterialManager::BindMaterial(const std::string& name, const std::shared_ptr<BaseShader>& shader) {
    auto mat = GetMaterial(name);
    if (!mat) {
        Logger::GetLogger()->error("Cannot bind unknown material '{}'.", name);
        return;
    }
    mat->Bind(shader);
    m_CurrentlyBoundMaterial = name;
}

void MaterialManager::UnbindMaterial() {
    if (!m_CurrentlyBoundMaterial.empty()) {
        auto mat = GetMaterial(m_CurrentlyBoundMaterial);
        if (mat) mat->Unbind();
        m_CurrentlyBoundMaterial.clear();
    }
}
