#pragma once

#include <memory>
#include <string_view>
#include <utility>
#include "Resources/MeshManager.h"
#include "Graphics/Textures/TextureManager.h"
#include "Graphics/Materials/MaterialManager.h"
#include "Graphics/Shaders/ShaderManager.h"

class ResourceManager {
public:
    static ResourceManager& GetInstance();

    std::pair<MeshLayout, MaterialLayout> GetLayoutsFromShader(const std::string& shaderName);

    // Access to specialized managers
    graphics::MeshManager& GetMeshManager();
    //ModelManager& GetModelManager();
    graphics::TextureManager& GetTextureManager();
    graphics::MaterialManager& GetMaterialManager();
    graphics::ShaderManager& GetShaderManager();

private:
    ResourceManager();
    ~ResourceManager() = default;

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
};