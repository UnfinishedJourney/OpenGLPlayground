#include "MaterialManager.h"
#include "Utilities/Logger.h"

MaterialManager& MaterialManager::GetInstance() {
    static MaterialManager instance;
    return instance;
}

std::shared_ptr<Material> MaterialManager::GetMaterial(const std::string& name) const {
    auto it = m_Materials.find(name);
    if (it != m_Materials.end()) {
        return it->second;
    }
    Logger::GetLogger()->warn("Material '{}' not found.", name);
    return nullptr;
}

void MaterialManager::AddMaterial(const std::string& name, const std::shared_ptr<Material>& material) {
    m_Materials[name] = material;
}

void MaterialManager::RemoveMaterial(const std::string& name) {
    m_Materials.erase(name);
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

std::shared_ptr<Material> MaterialManager::CreateMaterial(const std::string& name, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, float shininess) {
    auto mat = std::make_shared<Material>();
    mat->SetName(name);
    mat->SetParam(MaterialParamType::Ambient, ambient);
    mat->SetParam(MaterialParamType::Diffuse, diffuse);
    mat->SetParam(MaterialParamType::Specular, specular);
    mat->SetParam(MaterialParamType::Shininess, shininess);
    return mat;
}

void MaterialManager::InitializeStandardMaterials() {
    // Create Gold Material
    glm::vec3 goldAmbient(0.24725f, 0.1995f, 0.0745f);
    glm::vec3 goldDiffuse(0.75164f, 0.60648f, 0.22648f);
    glm::vec3 goldSpecular(0.628281f, 0.555802f, 0.366065f);
    float goldShininess = 51.2f;
    auto goldMaterial = CreateMaterial("Gold", goldAmbient, goldDiffuse, goldSpecular, goldShininess);
    AddMaterial("Gold", goldMaterial);

    // Create Silver Material
    glm::vec3 silverAmbient(0.19225f, 0.19225f, 0.19225f);
    glm::vec3 silverDiffuse(0.50754f, 0.50754f, 0.50754f);
    glm::vec3 silverSpecular(0.508273f, 0.508273f, 0.508273f);
    float silverShininess = 51.2f;
    auto silverMaterial = CreateMaterial("Silver", silverAmbient, silverDiffuse, silverSpecular, silverShininess);
    AddMaterial("Silver", silverMaterial);
}