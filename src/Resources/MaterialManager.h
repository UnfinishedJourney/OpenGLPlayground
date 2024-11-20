#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <string_view>
#include <filesystem>

#include "Graphics/Materials/Material.h"
#include "Graphics/Textures/TextureBase.h"
#include "Graphics/Textures/Texture2D.h"
#include "Graphics/Textures/CubeMapTexture.h"
#include "Graphics/Shaders/BaseShader.h"
#include "Utilities/Logger.h"

class MaterialManager {
public:
    MaterialManager() = default;
    ~MaterialManager() = default;

    std::shared_ptr<Material> GetMaterial(std::string_view materialName);

    // Separate methods for different texture types
    std::shared_ptr<Texture2D> GetTexture2D(std::string_view textureName);
    std::shared_ptr<CubeMapTexture> GetCubeMapTexture(std::string_view textureName);

    void AddMaterial(std::string_view name, const std::shared_ptr<Material>& material);
    void BindMaterial(std::string_view name, const std::shared_ptr<BaseShader>& shader);
    void UnbindMaterial();

private:
    std::string m_CurrentlyBoundMaterial;
    GLuint currentShaderID = 0;
    std::unordered_map<std::string, std::shared_ptr<Material>> m_Materials;
    std::unordered_map<std::string, std::shared_ptr<Texture2D>> m_Texture2Ds;
    std::unordered_map<std::string, std::shared_ptr<CubeMapTexture>> m_CubeMapTextures;
};