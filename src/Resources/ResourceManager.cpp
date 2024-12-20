#include "Resources/ResourceManager.h"

ResourceManager& ResourceManager::GetInstance() {
    static ResourceManager instance;
    return instance;
}

//need to make this better 
std::pair<MeshLayout, MaterialLayout> ResourceManager::getLayoutsFromShader(std::string shaderName)
{
    MeshLayout meshL;
    MaterialLayout matL;

    if (shaderName == "simplelights")
    {
        matL.params.insert(MaterialParamType::Ambient);
        matL.params.insert(MaterialParamType::Diffuse);
        matL.params.insert(MaterialParamType::Specular);
        matL.params.insert(MaterialParamType::Shininess);

        meshL = { true, true, false, false, {} };

        return { meshL, matL };
    }

    throw std::runtime_error("Trying to use the wrong shader for models.");
    return {};
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