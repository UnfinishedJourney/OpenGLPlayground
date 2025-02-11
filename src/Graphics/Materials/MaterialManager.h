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
     * @brief Manager for materials.
     *
     * Stores materials as unique_ptr, maps names to IDs, and provides
     * functions to bind/unbind materials.
     */
    class MaterialManager {
    public:
        static MaterialManager& GetInstance();

        /**
         * @brief Get a material ID by name.
         * @return std::optional<size_t> Material ID if found.
         */
        std::optional<std::size_t> GetMaterialIDByName(const std::string& name) const;

        /**
         * @brief Add a new material.
         * @return std::optional<size_t> New material ID, or std::nullopt on failure.
         */
        std::optional<std::size_t> AddMaterial(std::unique_ptr<Material> material);

        /**
         * @brief Remove a material by name.
         */
        void RemoveMaterialByName(const std::string& name);

        /**
         * @brief Remove a material by ID.
         */
        void RemoveMaterialByID(std::size_t id);

        /**
         * @brief Bind a material by name.
         */
        void BindMaterial(const std::string& name, const std::shared_ptr<BaseShader>& shader);

        /**
         * @brief Bind a material by ID.
         */
        void BindMaterial(std::size_t id, const std::shared_ptr<BaseShader>& shader);

        /**
         * @brief Unbind the currently bound material.
         */
        void UnbindMaterial();

        /**
         * @brief Initialize standard/built-in materials.
         */
        void InitializeStandardMaterials();

        /**
         * @brief Get read-only access to the materials.
         */
        const std::vector<std::unique_ptr<Material>>& GetMaterials() const;

    private:
        MaterialManager() = default;
        ~MaterialManager() = default;
        MaterialManager(const MaterialManager&) = delete;
        MaterialManager& operator=(const MaterialManager&) = delete;

        // Materials stored by index (some entries may be nullptr after removal).
        std::vector<std::unique_ptr<Material>> materials_;
        // Mapping from material name to index in the materials_ vector.
        std::unordered_map<std::string, std::size_t> name_to_index_;
        // Currently bound material ID.
        std::optional<std::size_t> currently_bound_material_id_;
    };

} // namespace Graphics