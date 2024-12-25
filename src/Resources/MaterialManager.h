#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "Graphics/Materials/MaterialLayout.h"
#include "Graphics/Materials/Material.h"

class MaterialManager {
public:
    // Singleton
    static MaterialManager& GetInstance();

    // CRUD Operations
    std::shared_ptr<Material> GetMaterialByName(const std::string& name) const;
    void AddMaterial(const std::string& name, const MaterialLayout& layout, const std::shared_ptr<Material>& material);
    void RemoveMaterial(const std::string& name);

    // Search Operations
    std::vector<std::shared_ptr<Material>> GetMaterialsByLayout(const MaterialLayout& layout) const;

    // Binding
    void BindMaterial(const std::string& name, const std::shared_ptr<BaseShader>& shader);
    void UnbindMaterial();

    // Initialization
    void InitializeStandardMaterials();

    // Utility
    std::vector<std::string> GetMaterialNames() const;

private:
    // Singleton
    MaterialManager() = default;
    ~MaterialManager() = default;
    MaterialManager(const MaterialManager&) = delete;
    MaterialManager& operator=(const MaterialManager&) = delete;

    // Storage
    struct MaterialEntry {
        std::string name;
        MaterialLayout layout;
        std::shared_ptr<Material> material;

        MaterialEntry(const std::string& name_, const MaterialLayout& layout_, const std::shared_ptr<Material>& material_)
            : name(name_), layout(layout_), material(material_) {}
    };

    std::vector<MaterialEntry> m_Materials;

    // Indexes
    std::unordered_map<std::string, size_t> m_NameToIndex;
    std::unordered_multimap<MaterialLayout, size_t, std::hash<MaterialLayout>> m_LayoutToIndices;

    // Currently bound material
    std::string m_CurrentlyBoundMaterial;

    // Helper to create material
    std::shared_ptr<Material> CreateMaterial(const std::string& name,
        const glm::vec3& ambient,
        const glm::vec3& diffuse,
        const glm::vec3& specular,
        float shininess,
        const glm::vec3& emissive);
};