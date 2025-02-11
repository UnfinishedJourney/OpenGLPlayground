#pragma once

#include <string>
#include <optional>
#include <unordered_map>
#include <vector>
#include <memory>

#include "Material.h"
#include "Graphics/Shaders/BaseShader.h"

namespace Graphics {

    /**
     * @brief Manager for materials stored as std::unique_ptr.
     *
     * Provides:
     *   - A lookup from name -> material ID (size_t)
     *   - Functions to bind/unbind materials by ID or name
     *   - Tracking of material names for debug/logging
     */
    class MaterialManager {
    public:
        static MaterialManager& GetInstance();

        /**
         * @brief Nullopt if failed
         */
        std::optional<size_t> GetMaterialIDByName(const std::string& name) const;
        std::optional<size_t> AddMaterial(std::unique_ptr<Material> material);

        /**
         * @brief Removes a material if it exists. Unbinds it if currently bound.
         */
        void RemoveMaterialByName(const std::string& name);
        void RemoveMaterialByID(size_t id);


        void BindMaterial(const std::string& name, const std::shared_ptr<BaseShader>& shader);
        void BindMaterial(size_t id, const std::shared_ptr<BaseShader>& shader);

        void UnbindMaterial();

        /**
         * @brief Optionally create some standard/built-in materials at startup.
         */
        void InitializeStandardMaterials();

        /**
         * @return Read-only access to the underlying vector of unique_ptr<Material>.
         *         For advanced usage that needs direct iteration over all materials.
         */
        const std::vector<std::unique_ptr<Material>>& GetMaterials() const {
            return m_Materials;
        }

    private:
        MaterialManager() = default;
        ~MaterialManager() = default;
        MaterialManager(const MaterialManager&) = delete;
        MaterialManager& operator=(const MaterialManager&) = delete;

    private:
        // Storage of materials: index => unique_ptr<Material>
        // Some entries can be nullptr if removed.
        std::vector<std::unique_ptr<Material>> m_Materials;

        // Maps material name => index into m_Materials
        std::unordered_map<std::string, size_t> m_NameToIndex;

        // Track which material is currently bound (by name).
        // (We could also track by ID; name is just more "friendly" for logs.)
        std::string m_CurrentlyBoundMaterial;
    };

} // namespace Graphics