#include "MaterialManager.h"
#include "Utilities/Logger.h"
#include <stdexcept>
#include <algorithm>

namespace Graphics {

    MaterialManager& MaterialManager::GetInstance() {
        static MaterialManager instance;
        return instance;
    }

    std::shared_ptr<Material> MaterialManager::GetMaterialByName(const std::string& name) const {
        auto it = m_NameToIndex.find(name);
        if (it == m_NameToIndex.end()) {
            Logger::GetLogger()->warn("[MaterialManager] Material '{}' not found.", name);
            return nullptr;
        }
        return m_Materials[it->second].MaterialPtr;
    }

    std::shared_ptr<Material> MaterialManager::GetMaterialByID(int id) const {
        auto it = m_IDToIndex.find(id);
        if (it == m_IDToIndex.end()) {
            Logger::GetLogger()->warn("[MaterialManager] Material with ID={} not found.", id);
            return nullptr;
        }
        return m_Materials[it->second].MaterialPtr;
    }

    void MaterialManager::AddMaterial(const std::shared_ptr<Material>& material) {
        if (!material) {
            Logger::GetLogger()->error("[MaterialManager] AddMaterial: null material pointer.");
            return;
        }
        const std::string& name = material->GetName();
        if (name.empty()) {
            Logger::GetLogger()->warn("[MaterialManager] AddMaterial: unnamed material!");
        }
        if (m_NameToIndex.find(name) != m_NameToIndex.end()) {
            Logger::GetLogger()->warn("[MaterialManager] Overriding material '{}'.", name);
            RemoveMaterialByName(name);
        }
        ++m_LastID;
        material->SetID(m_LastID);
        size_t newIndex = m_Materials.size();
        m_Materials.emplace_back(name, m_LastID, material);
        m_NameToIndex[name] = newIndex;
        m_IDToIndex[m_LastID] = newIndex;
        Logger::GetLogger()->info("[MaterialManager] Added material '{}' (ID={}).", name, m_LastID);
    }

    void MaterialManager::RemoveMaterialByName(const std::string& name) {
        auto it = m_NameToIndex.find(name);
        if (it == m_NameToIndex.end()) {
            Logger::GetLogger()->warn("[MaterialManager] RemoveMaterialByName: '{}' not found.", name);
            return;
        }
        size_t idx = it->second;
        int id = m_Materials[idx].ID;
        if (idx != m_Materials.size() - 1) {
            std::swap(m_Materials[idx], m_Materials.back());
            m_NameToIndex[m_Materials[idx].Name] = idx;
            m_IDToIndex[m_Materials[idx].ID] = idx;
        }
        m_Materials.pop_back();
        m_NameToIndex.erase(name);
        m_IDToIndex.erase(id);
        Logger::GetLogger()->info("[MaterialManager] Removed material '{}'.", name);
        if (m_CurrentlyBoundMaterial == name)
            UnbindMaterial();
    }

    void MaterialManager::RemoveMaterialByID(int id) {
        auto it = m_IDToIndex.find(id);
        if (it == m_IDToIndex.end()) {
            Logger::GetLogger()->warn("[MaterialManager] RemoveMaterialByID: ID={} not found.", id);
            return;
        }
        size_t idx = it->second;
        std::string name = m_Materials[idx].Name;
        if (idx != m_Materials.size() - 1) {
            std::swap(m_Materials[idx], m_Materials.back());
            m_NameToIndex[m_Materials[idx].Name] = idx;
            m_IDToIndex[m_Materials[idx].ID] = idx;
        }
        m_Materials.pop_back();
        m_NameToIndex.erase(name);
        m_IDToIndex.erase(id);
        Logger::GetLogger()->info("[MaterialManager] Removed material '{}' (ID={}).", name, id);
        if (m_CurrentlyBoundMaterial == name)
            UnbindMaterial();
    }

    void MaterialManager::BindMaterial(const std::string& name, const std::shared_ptr<BaseShader>& shader) {
        auto mat = GetMaterialByName(name);
        if (!mat) {
            Logger::GetLogger()->error("[MaterialManager] Cannot bind unknown material '{}'.", name);
            return;
        }
        mat->Bind(shader);
        m_CurrentlyBoundMaterial = name;
        Logger::GetLogger()->debug("[MaterialManager] Bound material '{}'.", name);
    }

    void MaterialManager::BindMaterial(int id, const std::shared_ptr<BaseShader>& shader) {
        auto mat = GetMaterialByID(id);
        if (!mat) {
            Logger::GetLogger()->error("[MaterialManager] Cannot bind unknown material with ID={}.", id);
            return;
        }
        mat->Bind(shader);
        m_CurrentlyBoundMaterial = mat->GetName();
        Logger::GetLogger()->debug("[MaterialManager] Bound material '{}' (ID={}).", mat->GetName(), id);
    }

    void MaterialManager::UnbindMaterial() {
        if (!m_CurrentlyBoundMaterial.empty()) {
            auto mat = GetMaterialByName(m_CurrentlyBoundMaterial);
            if (mat)
                mat->Unbind();
            m_CurrentlyBoundMaterial.clear();
            Logger::GetLogger()->debug("[MaterialManager] Unbound material.");
        }
    }

    std::vector<std::string> MaterialManager::GetMaterialNames() const {
        std::vector<std::string> names;
        names.reserve(m_Materials.size());
        for (const auto& entry : m_Materials) {
            names.push_back(entry.Name);
        }
        return names;
    }

    void MaterialManager::InitializeStandardMaterials() {
        // Example: Create built-in materials here.
        // For instance:
        //
        // MaterialLayout goldLayout({MaterialParamType::Ambient, MaterialParamType::Diffuse,
        //                              MaterialParamType::Specular, MaterialParamType::Shininess});
        // auto gold = std::make_shared<Material>(goldLayout);
        // gold->SetName("Gold");
        // gold->AssignToPackedParams(MaterialParamType::Ambient, glm::vec3(0.24725f, 0.1995f, 0.0745f));
        // gold->AssignToPackedParams(MaterialParamType::Diffuse, glm::vec3(0.75164f, 0.60648f, 0.22648f));
        // gold->AssignToPackedParams(MaterialParamType::Specular, glm::vec3(0.628281f, 0.555802f, 0.366065f));
        // gold->AssignToPackedParams(MaterialParamType::Shininess, 51.2f);
        // AddMaterial(gold);
    }

} // namespace Graphics