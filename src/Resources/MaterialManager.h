#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <string_view>
#include <filesystem>

#include "Graphics/Materials/Material.h"
#include "Graphics/Textures/Texture2D.h"
#include "Graphics/Shaders/BaseShader.h"
#include "Utilities/Logger.h"

class MaterialManager {
public:
    MaterialManager() = default;
    ~MaterialManager() = default;

    std::shared_ptr<Material> GetMaterial(std::string_view materialName);
    std::shared_ptr<Texture2D> GetTexture(std::string_view textureName);
    void AddMaterial(std::string_view name, std::shared_ptr<Material> material);
    void BindMaterial(std::string_view name, std::shared_ptr<BaseShader> shader);
    void UnbindMaterial(std::string_view name);

private:
    std::string m_CurrentlyBoundMaterial;
    std::unordered_map<std::string, std::shared_ptr<Material>> m_Materials;
    std::unordered_map<std::string, std::shared_ptr<Texture2D>> m_Textures;
};