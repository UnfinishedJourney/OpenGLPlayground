#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <glm/glm.hpp>
#include "Graphics/Materials/Material.h"
#include "Graphics/Materials/MaterialLayout.h"
#include "Graphics/Shaders/BaseShader.h"

/**
 * Manages a collection of Material objects by name.
 * For static geometry, you typically create them once
 * after loading from a model or from your own definitions.
 */
class MaterialManager {
public:
    // Singleton
    static MaterialManager& GetInstance();

    // CRUD
    std::shared_ptr<Material> GetMaterial(const std::string& name) const;
    void AddMaterial(const std::string& name, const std::shared_ptr<Material>& material);
    void RemoveMaterial(const std::string& name);

    // Binding
    void BindMaterial(const std::string& name, const std::shared_ptr<BaseShader>& shader);
    void UnbindMaterial();

    // Initialization
    void InitializeStandardMaterials();

    // Simple Factory
    std::shared_ptr<Material> CreateMaterial(const std::string& name,
        const glm::vec3& ambient,
        const glm::vec3& diffuse,
        const glm::vec3& specular,
        float shininess);

private:
    // Singleton
    MaterialManager() = default;
    ~MaterialManager() = default;
    MaterialManager(const MaterialManager&) = delete;
    MaterialManager& operator=(const MaterialManager&) = delete;

    std::unordered_map<std::string, std::shared_ptr<Material>> m_Materials;
    std::string m_CurrentlyBoundMaterial;
};