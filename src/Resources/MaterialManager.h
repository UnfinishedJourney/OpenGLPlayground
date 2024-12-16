#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include "Graphics/Materials/Material.h"
#include "Graphics/Shaders/BaseShader.h"

class MaterialManager {
public:
    static MaterialManager& GetInstance();

    std::shared_ptr<Material> GetMaterial(const std::string& name);
    void AddMaterial(const std::string& name, const std::shared_ptr<Material>& material);

    void BindMaterial(const std::string& name, const std::shared_ptr<BaseShader>& shader);
    void UnbindMaterial();

private:
    MaterialManager() = default;
    ~MaterialManager() = default;

    std::string m_CurrentlyBoundMaterial;
    std::unordered_map<std::string, std::shared_ptr<Material>> m_Materials;
};