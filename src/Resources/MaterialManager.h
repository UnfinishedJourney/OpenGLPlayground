#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include "Graphics/Materials/Material.h"
#include "Graphics/Shaders/BaseShader.h"

namespace Graphics {

    /**
     * @brief Manager for materials.
     *
     * Provides caching and retrieval of materials by name or ID,
     * and functions to bind/unbind materials.
     */
    class MaterialManager {
    public:
        static MaterialManager& GetInstance();

        std::shared_ptr<Material> GetMaterialByName(const std::string& name) const;
        std::shared_ptr<Material> GetMaterialByID(int id) const;

        void AddMaterial(const std::shared_ptr<Material>& material);
        void RemoveMaterialByName(const std::string& name);
        void RemoveMaterialByID(int id);

        void BindMaterial(const std::string& name, const std::shared_ptr<BaseShader>& shader);
        void BindMaterial(int id, const std::shared_ptr<BaseShader>& shader);
        void UnbindMaterial();

        std::vector<std::string> GetMaterialNames() const;
        void InitializeStandardMaterials();

    private:
        MaterialManager() = default;
        ~MaterialManager() = default;
        MaterialManager(const MaterialManager&) = delete;
        MaterialManager& operator=(const MaterialManager&) = delete;

        struct MaterialEntry {
            std::string Name;
            int ID;
            std::shared_ptr<Material> MaterialPtr;
            MaterialEntry(std::string nm, int mid, std::shared_ptr<Material> mat)
                : Name(std::move(nm)), ID(mid), MaterialPtr(std::move(mat)) {}
        };

        int m_LastID = 0;
        std::vector<MaterialEntry> m_Materials;
        std::unordered_map<std::string, size_t> m_NameToIndex;
        std::unordered_map<int, size_t> m_IDToIndex;

        std::string m_CurrentlyBoundMaterial;
    };

} // namespace Graphics