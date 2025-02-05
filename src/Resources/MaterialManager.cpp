#include "MaterialManager.h"
#include <stdexcept>

MaterialManager& MaterialManager::GetInstance()
{
    static MaterialManager instance;
    return instance;
}

std::shared_ptr<Material> MaterialManager::GetMaterialByName(const std::string& name) const
{
    auto it = m_NameToIndex.find(name);
    if (it == m_NameToIndex.end()) {
        Logger::GetLogger()->warn("[MaterialManager] Material '{}' not found.", name);
        return nullptr;
    }
    return m_Materials[it->second].material;
}

std::shared_ptr<Material> MaterialManager::GetMaterialByID(int id) const
{
    auto it = m_IDToIndex.find(id);
    if (it == m_IDToIndex.end()) {
        Logger::GetLogger()->warn("[MaterialManager] Material with ID={} not found.", id);
        return nullptr;
    }
    return m_Materials[it->second].material;
}

void MaterialManager::AddMaterial(const std::shared_ptr<Material>& material)
{
    if (!material) {
        Logger::GetLogger()->error("[MaterialManager] AddMaterial: null material pointer.");
        return;
    }

    const auto& name = material->GetName();
    if (name.empty()) {
        Logger::GetLogger()->warn("[MaterialManager] AddMaterial: unnamed material!");
    }

    // If name already exists, optionally remove or override
    if (auto it = m_NameToIndex.find(name); it != m_NameToIndex.end()) {
        Logger::GetLogger()->warn("[MaterialManager] Overriding material '{}'.", name);
        RemoveMaterialByName(name);
    }

    // Assign an ID
    ++m_LastId;
    material->SetID(m_LastId);

    // Insert
    size_t newIndex = m_Materials.size();
    m_Materials.emplace_back(name, m_LastId, material);
    m_NameToIndex[name] = newIndex;
    m_IDToIndex[m_LastId] = newIndex;

    Logger::GetLogger()->info("[MaterialManager] Added material '{}' (ID={}).", name, m_LastId);
}

void MaterialManager::RemoveMaterialByName(const std::string& name)
{
    auto it = m_NameToIndex.find(name);
    if (it == m_NameToIndex.end()) {
        Logger::GetLogger()->warn("[MaterialManager] RemoveMaterialByName: '{}' not found.", name);
        return;
    }

    size_t idx = it->second;
    auto id = m_Materials[idx].id;

    // If not the last element, swap with last
    if (idx != m_Materials.size() - 1) {
        // Perform swap
        std::swap(m_Materials[idx], m_Materials.back());
        // Update lookups for the swapped entry
        m_NameToIndex[m_Materials[idx].name] = idx;
        m_IDToIndex[m_Materials[idx].id] = idx;
    }

    // Pop back
    m_Materials.pop_back();
    m_NameToIndex.erase(name);
    m_IDToIndex.erase(id);

    Logger::GetLogger()->info("[MaterialManager] Removed material '{}'.", name);

    // If it was bound, unbind
    if (m_CurrentlyBoundMaterial == name) {
        UnbindMaterial();
    }
}

void MaterialManager::RemoveMaterialByID(int id)
{
    auto it = m_IDToIndex.find(id);
    if (it == m_IDToIndex.end()) {
        Logger::GetLogger()->warn("[MaterialManager] RemoveMaterialByID: ID={} not found.", id);
        return;
    }

    size_t idx = it->second;
    auto& entryName = m_Materials[idx].name;

    // If not last, swap
    if (idx != m_Materials.size() - 1) {
        std::swap(m_Materials[idx], m_Materials.back());
        m_NameToIndex[m_Materials[idx].name] = idx;
        m_IDToIndex[m_Materials[idx].id] = idx;
    }

    m_Materials.pop_back();
    m_NameToIndex.erase(entryName);
    m_IDToIndex.erase(id);

    Logger::GetLogger()->info("[MaterialManager] Removed material '{}'(ID={}).", entryName, id);

    if (m_CurrentlyBoundMaterial == entryName) {
        UnbindMaterial();
    }
}

void MaterialManager::BindMaterial(const std::string& name, const std::shared_ptr<BaseShader>& shader)
{
    auto mat = GetMaterialByName(name);
    if (!mat) {
        Logger::GetLogger()->error("[MaterialManager] Cannot bind unknown material '{}'.", name);
        return;
    }
    mat->Bind(shader);
    m_CurrentlyBoundMaterial = name;
    Logger::GetLogger()->debug("[MaterialManager] Bound material '{}'.", name);
}

void MaterialManager::BindMaterial(int id, const std::shared_ptr<BaseShader>& shader)
{
    auto mat = GetMaterialByID(id);
    if (!mat) {
        Logger::GetLogger()->error("[MaterialManager] Cannot bind unknown material with ID={}.", id);
        return;
    }
    mat->Bind(shader);
    m_CurrentlyBoundMaterial = mat->GetName();
    Logger::GetLogger()->debug("[MaterialManager] Bound material '{}'(ID={}).",
        mat->GetName(), id);
}

void MaterialManager::UnbindMaterial()
{
    if (!m_CurrentlyBoundMaterial.empty()) {
        auto mat = GetMaterialByName(m_CurrentlyBoundMaterial);
        if (mat) {
            mat->Unbind();
        }
        m_CurrentlyBoundMaterial.clear();
        Logger::GetLogger()->debug("[MaterialManager] Unbound material.");
    }
}

std::vector<std::string> MaterialManager::GetMaterialNames() const
{
    std::vector<std::string> names;
    names.reserve(m_Materials.size());
    for (const auto& entry : m_Materials) {
        names.push_back(entry.name);
    }
    return names;
}

void MaterialManager::InitializeStandardMaterials()
{
    // Example: create or load built-in materials here.
    // For instance:
    //
    // auto goldLayout = MaterialLayout{
    //     {MaterialParamType::Ambient, MaterialParamType::Diffuse,
    //      MaterialParamType::Specular, MaterialParamType::Shininess},
    //     {}
    // };
    // auto gold = std::make_shared<Material>(goldLayout);
    // gold->SetName("Gold");
    // gold->AssignToPackedParams(MaterialParamType::Ambient, glm::vec3(0.24725f, 0.1995f, 0.0745f));
    // gold->AssignToPackedParams(MaterialParamType::Diffuse, glm::vec3(0.75164f, 0.60648f, 0.22648f));
    // gold->AssignToPackedParams(MaterialParamType::Specular,glm::vec3(0.628281f,0.555802f,0.366065f));
    // gold->AssignToPackedParams(MaterialParamType::Shininess, 51.2f);
    // AddMaterial(gold);
}