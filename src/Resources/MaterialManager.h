#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <string_view>
#include <filesystem>

#include "Graphics/Materials/Material.h"
#include "Graphics/Textures/TextureBase.h"
#include "Graphics/Shaders/BaseShader.h"
#include "Utilities/Logger.h"

class MaterialManager {
public:
    MaterialManager() = default;
    ~MaterialManager() = default;

    std::shared_ptr<Material> GetMaterial(std::string_view materialName);
    std::shared_ptr<TextureBase> GetTexture(std::string_view textureName);
    void AddMaterial(std::string_view name, const std::shared_ptr<Material>& material);
    void BindMaterial(std::string_view name, const std::shared_ptr<BaseShader>& shader);
    void UnbindMaterial();

private:
    std::string m_CurrentlyBoundMaterial;
    GLuint currentShaderID = 0;
    std::unordered_map<std::string, std::shared_ptr<Material>> m_Materials;
    std::unordered_map<std::string, std::shared_ptr<TextureBase>> m_Textures;
};