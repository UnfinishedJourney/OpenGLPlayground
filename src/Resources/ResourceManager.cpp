#include "Resources/ResourceManager.h"

ResourceManager& ResourceManager::GetInstance() {
    static ResourceManager instance;
    return instance;
}

//need to make this better 
std::pair<MeshLayout, MaterialLayout> ResourceManager::GetLayoutsFromShader(const std::string& shaderName)
{
    static const std::unordered_map<std::string, std::pair<MeshLayout, MaterialLayout>> shaderLayouts = {
        {"simplelights", {
            {true, true, false, false, {}},
            {{MaterialParamType::Ambient, MaterialParamType::Diffuse, MaterialParamType::Specular, MaterialParamType::Shininess}, {}}
        }},
        {"basic", {
            {true, false, false, false, {TextureType::Albedo}},
            {{}, {}}
        }},
        {"basicTextured", {
            {true, false, false, false, {TextureType::Albedo}},
            {{}, {TextureType::Albedo}}
        }},
        {"helmetPBR", {
            {true, true, true, false, {TextureType::Albedo}},
            {{}, {TextureType::Albedo, TextureType::Normal, TextureType::MetalRoughness, TextureType::AO, TextureType::Emissive, TextureType::BRDFLut}}
        }},
        {"bistroShader", {
            {true, true, false, false, {TextureType::Albedo}},
            {{}, {TextureType::Albedo, TextureType::Ambient}}
        }},
        {"bistroShaderShadowed", {
            {true, true, false, false, {TextureType::Albedo}},
            {{}, {TextureType::Albedo, TextureType::Normal, TextureType::MetalRoughness, TextureType::AO, TextureType::Emissive, TextureType::BRDFLut, TextureType::Ambient}}
        }},
        {"simpleLightsShadowed", {
            {true, true, false, false, {}},
            {{MaterialParamType::Ambient, MaterialParamType::Diffuse, MaterialParamType::Specular, MaterialParamType::Shininess}, {}}
        }},
    };

    try {
        return shaderLayouts.at(shaderName);
    }
    catch (const std::out_of_range&) {
        throw std::runtime_error("Trying to use the wrong shader for models: " + shaderName);
    }
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

Graphics::TextureManager& ResourceManager::GetTextureManager() {
    return Graphics::TextureManager::GetInstance();
}

Graphics::MaterialManager& ResourceManager::GetMaterialManager() {
    return Graphics::MaterialManager::GetInstance();
}

Graphics::ShaderManager& ResourceManager::GetShaderManager() {
    return Graphics::ShaderManager::GetInstance();
}