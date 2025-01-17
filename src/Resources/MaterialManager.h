#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include "Graphics/Materials/Material.h"
#include "Graphics/Materials/MaterialBuilder.h"
#include "Graphics/Materials/MaterialLayout.h"
#include "Graphics/Shaders/BaseShader.h"
#include "Utilities/Logger.h"

/**
 * @brief A manager to handle all Materials by both name and ID.
 */
class MaterialManager
{
public:
    static MaterialManager& GetInstance();

    std::shared_ptr<Material> GetMaterialByName(const std::string& name) const;
    void AddMaterial(const std::shared_ptr<Material>& material);
    void RemoveMaterialByName(const std::string& name);

    std::shared_ptr<Material> GetMaterialByID(MaterialID id) const;
    void RemoveMaterialByID(MaterialID id);

    // Binding
    void BindMaterial(const std::string& name, const std::shared_ptr<BaseShader>& shader);
    void UnbindMaterial();

    //Pre-load some standard materials
    void InitializeStandardMaterials();

    // Utility
    std::vector<std::string> GetMaterialNames() const;

private:
    MaterialManager() = default;
    ~MaterialManager() = default;
    MaterialManager(const MaterialManager&) = delete;
    MaterialManager& operator=(const MaterialManager&) = delete;

    MaterialID m_LastId = 0;
    // For internal storage
    struct MaterialEntry {
        std::string name;
        MaterialID  id;
        MaterialLayout layout;
        std::shared_ptr<Material> material;

        MaterialEntry(const std::string& name_,
            MaterialID id_,
            const MaterialLayout& layout_,
            const std::shared_ptr<Material>& mat_)
            : name(name_), id(id_), layout(layout_), material(mat_) {}
    };

    // Storage
    std::vector<MaterialEntry> m_Materials;

    // Lookup maps
    std::unordered_map<std::string, size_t> m_NameToIndex;
    std::unordered_map<MaterialID, size_t>  m_IDToIndex;

    // Currently bound material
    std::string m_CurrentlyBoundMaterial;
};