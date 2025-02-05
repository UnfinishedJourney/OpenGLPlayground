#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include "Graphics/Materials/Material.h"
#include "Graphics/Shaders/BaseShader.h"
#include "Utilities/Logger.h"

/**
 * @brief A manager to handle all Materials by both name and ID.
 *
 * The typical usage is:
 *   - Create a Material
 *   - Add it via AddMaterial()
 *   - Then retrieve by name or ID and call BindMaterial(...).
 */
class MaterialManager
{
public:
    static MaterialManager& GetInstance();

    // Retrieve by name or ID
    std::shared_ptr<Material> GetMaterialByName(const std::string& name) const;
    std::shared_ptr<Material> GetMaterialByID(int id) const;

    // Add or remove
    void AddMaterial(const std::shared_ptr<Material>& material);
    void RemoveMaterialByName(const std::string& name);
    void RemoveMaterialByID(int id);

    // Bind / Unbind by name or ID
    void BindMaterial(const std::string& name, const std::shared_ptr<BaseShader>& shader);
    void BindMaterial(int id, const std::shared_ptr<BaseShader>& shader);
    void UnbindMaterial();

    // Utility
    std::vector<std::string> GetMaterialNames() const;
    void InitializeStandardMaterials();

private:
    MaterialManager() = default;
    ~MaterialManager() = default;

    MaterialManager(const MaterialManager&) = delete;
    MaterialManager& operator=(const MaterialManager&) = delete;

private:
    // Each new material gets an auto-increment ID
    int m_LastId = 0;

    // Internal storage item
    struct MaterialEntry {
        std::string name;
        int  id;
        std::shared_ptr<Material> material;

        MaterialEntry(std::string nm, int mid, std::shared_ptr<Material> mat)
            : name(std::move(nm))
            , id(mid)
            , material(std::move(mat))
        {}
    };

    // Storage
    std::vector<MaterialEntry> m_Materials;

    // Lookups
    std::unordered_map<std::string, size_t> m_NameToIndex;
    std::unordered_map<int, size_t>  m_IDToIndex;

    // Currently bound material name
    std::string m_CurrentlyBoundMaterial;
};