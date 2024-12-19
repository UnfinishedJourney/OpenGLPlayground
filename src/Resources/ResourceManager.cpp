#include "Resources/ResourceManager.h"

ResourceManager& ResourceManager::GetInstance() {
    static ResourceManager instance;
    return instance;
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