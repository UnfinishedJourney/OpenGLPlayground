#include "MaterialManager.h"
#include "Utilities/Logger.h"
#include <stdexcept>

namespace graphics {

    MaterialManager& MaterialManager::GetInstance() {
        static MaterialManager instance;
        return instance;
    }

    std::optional<std::size_t> MaterialManager::GetMaterialIDByName(const std::string& name) const {
        auto it = name_to_index_.find(name);
        if (it != name_to_index_.end())
            return it->second;
        return std::nullopt;
    }

    std::optional<std::size_t> MaterialManager::AddMaterial(std::unique_ptr<Material> material) {
        if (!material) {
            Logger::GetLogger()->error("[MaterialManager] AddMaterial: null material pointer.");
            return std::nullopt;
        }
        const std::string& name = material->GetName();
        if (name.empty()) {
            Logger::GetLogger()->warn("[MaterialManager] AddMaterial: unnamed material!");
        }

        // If a material with the same name exists, remove it.
        if (!name.empty()) {
            auto existing = name_to_index_.find(name);
            if (existing != name_to_index_.end()) {
                Logger::GetLogger()->warn("[MaterialManager] Overriding material '{}'.", name);
                RemoveMaterialByName(name);
            }
        }

        std::size_t newIndex = materials_.size();
        material->SetID(newIndex);
        materials_.emplace_back(std::move(material));
        if (!name.empty()) {
            name_to_index_[materials_[newIndex]->GetName()] = newIndex;
        }
        Logger::GetLogger()->info("[MaterialManager] Added material '{}' (ID={}).",
            materials_[newIndex]->GetName(), newIndex);
        return newIndex;
    }

    void MaterialManager::RemoveMaterialByName(const std::string& name) {
        auto it = name_to_index_.find(name);
        if (it == name_to_index_.end()) {
            Logger::GetLogger()->warn("[MaterialManager] RemoveMaterialByName: '{}' not found.", name);
            return;
        }
        std::size_t idx = it->second;
        // Unbind if this material is currently bound.
        if (currently_bound_material_id_.has_value() && currently_bound_material_id_.value() == idx) {
            UnbindMaterial();
        }
        materials_[idx] = nullptr;
        name_to_index_.erase(it);
    }

    void MaterialManager::RemoveMaterialByID(std::size_t id) {
        if (id >= materials_.size() || !materials_[id]) {
            Logger::GetLogger()->warn("[MaterialManager] Material with ID={} not found or already removed.", id);
            return;
        }
        std::string name = materials_[id]->GetName();
        RemoveMaterialByName(name);
    }

    void MaterialManager::BindMaterial(const std::string& name, const std::shared_ptr<BaseShader>& shader) {
        auto maybeID = GetMaterialIDByName(name);
        if (!maybeID.has_value()) {
            Logger::GetLogger()->error("[MaterialManager] Cannot bind unknown material '{}'.", name);
            return;
        }
        BindMaterial(maybeID.value(), shader);
    }

    void MaterialManager::BindMaterial(std::size_t id, const std::shared_ptr<BaseShader>& shader) {
        if (id >= materials_.size() || !materials_[id]) {
            Logger::GetLogger()->error("[MaterialManager] Cannot bind unknown material with ID={}.", id);
            return;
        }
        materials_[id]->Bind(shader);
        currently_bound_material_id_ = id;
        Logger::GetLogger()->debug("[MaterialManager] Bound material '{}' (ID={}).",
            materials_[id]->GetName(), id);
    }

    void MaterialManager::UnbindMaterial() {
        if (currently_bound_material_id_.has_value()) {
            std::size_t id = currently_bound_material_id_.value();
            if (id < materials_.size() && materials_[id]) {
                materials_[id]->Unbind();
            }
            currently_bound_material_id_.reset();
            Logger::GetLogger()->debug("[MaterialManager] Unbound material.");
        }
    }

    void MaterialManager::InitializeStandardMaterials() {
        // Create and add built-in materials here.
        // For example:
        // MaterialLayout layout({MaterialParamType::Ambient, MaterialParamType::Diffuse},
        //                       {TextureType::Albedo, TextureType::Normal});
        // auto gold = std::make_unique<Material>(layout);
        // gold->SetName("Gold");
        // gold->AssignToPackedParams(MaterialParamType::Ambient, glm::vec3(1.0f, 0.8f, 0.0f));
        // AddMaterial(std::move(gold));
    }

    const std::vector<std::unique_ptr<Material>>& MaterialManager::GetMaterials() const {
        return materials_;
    }

} // namespace graphics