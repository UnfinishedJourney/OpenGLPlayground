#pragma once

#include <memory>
#include <string_view>

#include "Resources/MeshManager.h"
#include "Resources/ModelManager.h"
#include "Resources/TextureManager.h"
#include "Resources/MaterialManager.h"
#include "Resources/ShaderManager.h"

class ResourceManager {
public:
    static ResourceManager& GetInstance();

    // Access to specialized managers
    MeshManager& GetMeshManager();
    ModelManager& GetModelManager();
    TextureManager& GetTextureManager();
    MaterialManager& GetMaterialManager();
    ShaderManager& GetShaderManager();

private:
    ResourceManager();
    ~ResourceManager() = default;

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
};