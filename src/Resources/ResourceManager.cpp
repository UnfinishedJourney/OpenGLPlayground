#include "Resources/ResourceManager.h"

ResourceManager& ResourceManager::GetInstance() {
    static ResourceManager instance;
    return instance;
}

//need to make this better 
std::pair<MeshLayout, MaterialLayout> ResourceManager::getLayoutsFromShader(std::string shaderName)
{
    static const std::unordered_map<std::string, std::pair<MeshLayout, MaterialLayout>> shaderLayouts = {
        {"simplelights",
            {
                MeshLayout{ true, true, false, false, {} },
                MaterialLayout{ {MaterialParamType::Ambient, MaterialParamType::Diffuse, MaterialParamType::Specular, MaterialParamType::Shininess}, {} }
            }
        },
        {"basic",
            {
                MeshLayout{ true, false, false, false, {TextureType::Albedo} },
                MaterialLayout{ {}, {} }
            }
        },
        // Add more shader layouts as needed
    };

    auto it = shaderLayouts.find(shaderName);
    if (it != shaderLayouts.end()) {
        return it->second;
    }

    throw std::runtime_error("Trying to use the wrong shader for models.");
}

ResourceManager::ResourceManager() {
    // Initialization if needed
}

MeshManager& ResourceManager::GetMeshManager() {
    return MeshManager::GetInstance();
}

//ModelManager& ResourceManager::GetModelManager() {
//    return ModelManager::GetInstance();
//}

TextureManager& ResourceManager::GetTextureManager() {
    return TextureManager::GetInstance();
}

MaterialManager& ResourceManager::GetMaterialManager() {
    return MaterialManager::GetInstance();
}

ShaderManager& ResourceManager::GetShaderManager() {
    return ShaderManager::GetInstance();
}