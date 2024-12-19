#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include "Graphics/Materials/Material.h"
#include "Graphics/Materials/MaterialLayout.h"
#include "Graphics/Shaders/BaseShader.h"

class MaterialManager {
public:
    // Singleton Access
    static MaterialManager& GetInstance();

    // Material Management
    std::shared_ptr<Material> GetMaterial(const std::string& name) const;
    void AddMaterial(const std::string& name, const std::shared_ptr<Material>& material);
    void RemoveMaterial(const std::string& name);

    // Binding
    void BindMaterial(const std::string& name, const std::shared_ptr<BaseShader>& shader);
    void UnbindMaterial();

    // Initialization
    void InitializeStandardMaterials();

    // Factory Method
    std::shared_ptr<Material> CreateMaterial(const std::string& name, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, float shininess);
private:
    // Private Constructor for Singleton
    MaterialManager() = default;
    ~MaterialManager() = default;

    // Disable copy and assignment
    MaterialManager(const MaterialManager&) = delete;
    MaterialManager& operator=(const MaterialManager&) = delete;

    std::string m_CurrentlyBoundMaterial;
    std::unordered_map<std::string, std::shared_ptr<Material>> m_Materials;

};