#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <string_view>

#include "Graphics/Materials/Material.h"
#include "Graphics/Shaders/BaseShader.h"

class MaterialManager {
public:
    static MaterialManager& GetInstance();

    std::shared_ptr<Material> GetMaterial(std::string_view materialName);
    void AddMaterial(std::string_view name, const std::shared_ptr<Material>& material);
    void BindMaterial(std::string_view name, const std::shared_ptr<BaseShader>& shader);
    void UnbindMaterial();

private:
    MaterialManager() = default;
    ~MaterialManager() = default;

    MaterialManager(const MaterialManager&) = delete;
    MaterialManager& operator=(const MaterialManager&) = delete;

    std::string m_CurrentlyBoundMaterial;
    std::unordered_map<std::string, std::shared_ptr<Material>> m_Materials;
};