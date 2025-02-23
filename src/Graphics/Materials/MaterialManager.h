#pragma once
#include <string>
#include <optional>
#include <unordered_map>
#include <vector>
#include <memory>
#include "Material.h"
#include "Graphics/Shaders/BaseShader.h"

namespace graphics {

    /**
     * @brief Manager for materials.
     *
     * Stores materials and provides functions to bind/unbind them.
     */
    class MaterialManager {
    public:
        static MaterialManager& GetInstance();

        std::optional<std::size_t> GetMaterialIDByName(const std::string& name) const;
        std::optional<std::size_t> AddMaterial(std::unique_ptr<Material> material);
        void RemoveMaterialByName(const std::string& name);
        void RemoveMaterialByID(std::size_t id);
        void BindMaterial(const std::string& name, const std::shared_ptr<BaseShader>& shader);
        void BindMaterial(std::size_t id, const std::shared_ptr<BaseShader>& shader);
        void UnbindMaterial();
        void InitializeStandardMaterials();
        const std::vector<std::unique_ptr<Material>>& GetMaterials() const;

    private:
        MaterialManager() = default;
        ~MaterialManager() = default;
        MaterialManager(const MaterialManager&) = delete;
        MaterialManager& operator=(const MaterialManager&) = delete;

        std::vector<std::unique_ptr<Material>> materials_;
        std::unordered_map<std::string, std::size_t> nameToIndex_;
        std::optional<std::size_t> currentlyBoundMaterialId_;
    };

} // namespace graphics