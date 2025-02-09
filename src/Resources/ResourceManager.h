#pragma once

#include <memory>
#include <string_view>
#include <utility>
#include "Resources/MeshManager.h"
#include "Graphics/Textures/TextureManager.h"
#include "Resources/MaterialManager.h"
#include "Resources/ShaderManager.h"

class ResourceManager {
public:
    static ResourceManager& GetInstance();

    std::pair<MeshLayout, MaterialLayout> getLayoutsFromShader(std::string shaderName);

    // Access to specialized managers
    MeshManager& GetMeshManager();
    //ModelManager& GetModelManager();
    Graphics::TextureManager& GetTextureManager();
    MaterialManager& GetMaterialManager();
    ShaderManager& GetShaderManager();

private:
    ResourceManager();
    ~ResourceManager() = default;

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
};