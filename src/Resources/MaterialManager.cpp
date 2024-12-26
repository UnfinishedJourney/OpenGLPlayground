#include "MaterialManager.h"
#include "Graphics/Shaders/BaseShader.h"
#include "Utilities/Logger.h"
#include "Resources/TextureManager.h"

MaterialManager& MaterialManager::GetInstance() {
    static MaterialManager instance;
    return instance;
}

std::shared_ptr<Material> MaterialManager::GetMaterialByName(const std::string& name) const {
    auto it = m_NameToIndex.find(name);
    if (it != m_NameToIndex.end()) {
        return m_Materials[it->second].material;
    }
    Logger::GetLogger()->warn("Material '{}' not found.", name);
    return nullptr;
}

void MaterialManager::AddMaterial(const std::string& name, const MaterialLayout& layout, const std::shared_ptr<Material>& material) {
    if (m_NameToIndex.find(name) != m_NameToIndex.end()) {
        Logger::GetLogger()->warn("Material '{}' already exists. Overwriting.", name);
        size_t index = m_NameToIndex[name];
        m_Materials[index].layout = layout;
        m_Materials[index].material = material;

        // Note: If the layout changes, you should update the layout multimap accordingly
        // For simplicity, assuming layout is immutable after creation
        return;
    }

    m_Materials.emplace_back(name, layout, material);
    size_t index = m_Materials.size() - 1;
    m_NameToIndex[name] = index;
    m_LayoutToIndices.emplace(layout, index);
}

void MaterialManager::RemoveMaterial(const std::string& name) {
    auto it = m_NameToIndex.find(name);
    if (it == m_NameToIndex.end()) {
        Logger::GetLogger()->warn("Attempted to remove non-existent material '{}'.", name);
        return;
    }

    size_t index = it->second;
    MaterialLayout layout = m_Materials[index].layout;

    // Remove from layout multimap
    auto range = m_LayoutToIndices.equal_range(layout);
    for (auto itr = range.first; itr != range.second; ++itr) {
        if (itr->second == index) {
            m_LayoutToIndices.erase(itr);
            break;
        }
    }

    // Remove from vector
    m_Materials.erase(m_Materials.begin() + index);

    // Update indexes
    m_NameToIndex.erase(it);
    // Update indices in m_NameToIndex and m_LayoutToIndices
    for (size_t i = index; i < m_Materials.size(); ++i) {
        m_NameToIndex[m_Materials[i].name] = i;

        // Update layout multimap
        auto range_update = m_LayoutToIndices.equal_range(m_Materials[i].layout);
        for (auto itr = range_update.first; itr != range_update.second; ++itr) {
            if (itr->second == i + 1) { // Previous index was i + 1
                itr->second = i;
                break;
            }
        }
    }

    // Handle currently bound material
    if (m_CurrentlyBoundMaterial == name) {
        UnbindMaterial();
    }
}

std::vector<std::shared_ptr<Material>> MaterialManager::GetMaterialsByLayout(const MaterialLayout& layout) const {
    std::vector<std::shared_ptr<Material>> result;
    auto range = m_LayoutToIndices.equal_range(layout);
    for (auto it = range.first; it != range.second; ++it) {
        result.push_back(m_Materials[it->second].material);
    }
    return result;
}

void MaterialManager::BindMaterial(const std::string& name, const std::shared_ptr<BaseShader>& shader) {
    auto mat = GetMaterialByName(name);
    if (!mat) {
        Logger::GetLogger()->error("Cannot bind unknown material '{}'.", name);
        return;
    }
    mat->Bind(shader);
    m_CurrentlyBoundMaterial = name;
}

void MaterialManager::UnbindMaterial() {
    if (!m_CurrentlyBoundMaterial.empty()) {
        auto mat = GetMaterialByName(m_CurrentlyBoundMaterial);
        if (mat) mat->Unbind();
        m_CurrentlyBoundMaterial.clear();
    }
}

void MaterialManager::InitializeStandardMaterials() {
    // Example: Gold
    MaterialLayout goldLayout = {
        {MaterialParamType::Ambient, MaterialParamType::Diffuse, MaterialParamType::Specular, MaterialParamType::Shininess},
        {} // Add textures if any
    };

    glm::vec3 goldAmbient(0.24725f, 0.1995f, 0.0745f);
    glm::vec3 goldDiffuse(0.75164f, 0.60648f, 0.22648f);
    glm::vec3 goldSpecular(0.628281f, 0.555802f, 0.366065f);
    float goldShininess = 51.2f;

    auto gold = CreateMaterial("Gold", goldAmbient, goldDiffuse, goldSpecular, goldShininess, glm::vec3(0.0f));
    AddMaterial("Gold", goldLayout, gold);

    // Example: Silver
    MaterialLayout silverLayout = {
        {MaterialParamType::Ambient, MaterialParamType::Diffuse, MaterialParamType::Specular, MaterialParamType::Shininess},
        {} // Add textures if any
    };

    glm::vec3 silverAmbient(0.19225f, 0.19225f, 0.19225f);
    glm::vec3 silverDiffuse(0.50754f, 0.50754f, 0.50754f);
    glm::vec3 silverSpecular(0.508273f, 0.508273f, 0.508273f);
    float silverShininess = 51.2f;

    auto silver = CreateMaterial("Silver", silverAmbient, silverDiffuse, silverSpecular, silverShininess, glm::vec3(0.0f));
    AddMaterial("Silver", silverLayout, silver);

    // Add more standard materials as needed
}

std::vector<std::string> MaterialManager::GetMaterialNames() const {
    std::vector<std::string> names;
    names.reserve(m_Materials.size());
    for (const auto& entry : m_Materials) {
        names.emplace_back(entry.name);
    }
    return names;
}

std::shared_ptr<Material> MaterialManager::CreateMaterial(const std::string& name,
    const glm::vec3& ambient,
    const glm::vec3& diffuse,
    const glm::vec3& specular,
    float shininess,
    const glm::vec3& emissive)
{
    auto mat = std::make_shared<Material>();
    mat->SetName(name);
    mat->SetParam(MaterialParamType::Ambient, ambient);
    mat->SetParam(MaterialParamType::Diffuse, diffuse);
    mat->SetParam(MaterialParamType::Specular, specular);
    mat->SetParam(MaterialParamType::Shininess, shininess);
    return mat;
}