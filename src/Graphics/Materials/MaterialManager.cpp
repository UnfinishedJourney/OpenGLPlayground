#include "MaterialManager.h"
#include "Utilities/Logger.h"
#include <stdexcept>
#include <algorithm>

namespace Graphics {

    MaterialManager& MaterialManager::GetInstance() {
        static MaterialManager instance;
        return instance;
    }

    std::optional<size_t> MaterialManager::GetMaterialIDByName(const std::string& name) const {
        auto it = m_NameToIndex.find(name);
        if (it == m_NameToIndex.end()) {
            return std::nullopt;
        }
        return it->second;
    }

    std::optional<size_t> MaterialManager::AddMaterial(std::unique_ptr<Material> material) {
        if (!material) {
            Logger::GetLogger()->error("[MaterialManager] AddMaterial: null material pointer.");
            return std::nullopt; // fail
        }
        // We allow unnamed materials but log a warning
        const std::string& name = material->GetName();
        if (name.empty()) {
            Logger::GetLogger()->warn("[MaterialManager] AddMaterial: unnamed material!");
        }

        // If a material of this name already exists, remove it
        auto existing = m_NameToIndex.find(name);
        if (existing != m_NameToIndex.end()) {
            Logger::GetLogger()->warn("[MaterialManager] Overriding material '{}'.", name);
            RemoveMaterialByName(name);
        }

        // The new material's ID is just the next index in our vector
        size_t newIndex = m_Materials.size();
        material->SetID(newIndex);

        // Move it in
        m_Materials.emplace_back(std::move(material));
        m_NameToIndex[name] = newIndex;

        Logger::GetLogger()->info("[MaterialManager] Added material '{}' (ID={}).",
            name, newIndex);
        return newIndex;
    }

    void MaterialManager::RemoveMaterialByName(const std::string& name) {
        auto it = m_NameToIndex.find(name);
        if (it == m_NameToIndex.end()) {
            Logger::GetLogger()->warn("[MaterialManager] RemoveMaterialByName: '{}' not found.", name);
            return;
        }
        // If it's currently bound, unbind it
        if (m_CurrentlyBoundMaterial == name) {
            UnbindMaterial();
        }

        // Mark the material slot as removed
        size_t idx = it->second;
        m_Materials[idx] = nullptr;
        m_NameToIndex.erase(it);
    }

    void MaterialManager::RemoveMaterialByID(size_t id) {
        if (id >= m_Materials.size() || !m_Materials[id]) {
            Logger::GetLogger()->warn("[MaterialManager] Material with ID={} not found or already removed.", id);
            return;
        }
        // Remove by name (this also unbinds if needed).
        std::string theName = m_Materials[id]->GetName();
        RemoveMaterialByName(theName);
    }

    void MaterialManager::BindMaterial(const std::string& name, const std::shared_ptr<BaseShader>& shader) {
        auto maybeID = GetMaterialIDByName(name);
        if (!maybeID.has_value()) {
            Logger::GetLogger()->error("[MaterialManager] Cannot bind unknown material '{}'.", name);
            return;
        }
        size_t id = maybeID.value();

        // Double-check the pointer
        if (!m_Materials[id]) {
            Logger::GetLogger()->error("[MaterialManager] Material ID={} is null? (Name='{}')", id, name);
            return;
        }
        m_Materials[id]->Bind(shader);
        m_CurrentlyBoundMaterial = name;

        Logger::GetLogger()->debug("[MaterialManager] Bound material '{}'.", name);
    }

    void MaterialManager::BindMaterial(size_t id, const std::shared_ptr<BaseShader>& shader) {
        if (id >= m_Materials.size() || !m_Materials[id]) {
            Logger::GetLogger()->error("[MaterialManager] Cannot bind unknown material with ID={}.", id);
            return;
        }
        m_Materials[id]->Bind(shader);
        m_CurrentlyBoundMaterial = m_Materials[id]->GetName();

        Logger::GetLogger()->debug("[MaterialManager] Bound material '{}' (ID={}).",
            m_CurrentlyBoundMaterial, id);
    }

    void MaterialManager::UnbindMaterial() {
        if (!m_CurrentlyBoundMaterial.empty()) {
            // If we still have that material, call Unbind on it
            auto maybeID = GetMaterialIDByName(m_CurrentlyBoundMaterial);
            if (maybeID.has_value()) {
                size_t id = maybeID.value();
                if (id < m_Materials.size() && m_Materials[id]) {
                    m_Materials[id]->Unbind();
                }
            }
            m_CurrentlyBoundMaterial.clear();
            Logger::GetLogger()->debug("[MaterialManager] Unbound material.");
        }
    }

    void MaterialManager::InitializeStandardMaterials() {
        // Example if you want built-in materials.
        // E.g. auto gold = std::make_unique<Material>(someLayout);
        // gold->SetName("Gold");
        // ...
        // AddMaterial(std::move(gold));
    }

} // namespace Graphics