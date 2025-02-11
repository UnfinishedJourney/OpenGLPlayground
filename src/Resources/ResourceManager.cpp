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
            MeshLayout{ true, true, false, false, {} },
            MaterialLayout{ { MaterialParamType::Ambient, MaterialParamType::Diffuse, MaterialParamType::Specular, MaterialParamType::Shininess }, {} }
        }},
        {"basic", {
            MeshLayout{ true, false, false, false, { TextureType::Albedo } },
            MaterialLayout{ {}, {} }
        }},
        {"basicTextured", {
            MeshLayout{ true, false, false, false, { TextureType::Albedo } },
            MaterialLayout{ {}, { TextureType::Albedo } }
        }},
        {"helmetPBR", {
            MeshLayout{ true, true, true, false, { TextureType::Albedo } },
            MaterialLayout{ {}, { TextureType::Albedo, TextureType::Normal, TextureType::MetalRoughness, TextureType::AO, TextureType::Emissive, TextureType::BRDFLut } }
        }},
        {"bistroShader", {
            MeshLayout{ true, true, false, false, { TextureType::Albedo } },
            MaterialLayout{ {}, { TextureType::Albedo, TextureType::Ambient } }
        }},
        {"bistroShaderShadowed", {
            MeshLayout{ true, true, false, false, { TextureType::Albedo } },
            MaterialLayout{ {}, { TextureType::Albedo, TextureType::Normal, TextureType::MetalRoughness, TextureType::AO, TextureType::Emissive, TextureType::BRDFLut, TextureType::Ambient } }
        }},
        {"simpleLightsShadowed", {
            MeshLayout{ true, true, false, false, {} },
            MaterialLayout{ { MaterialParamType::Ambient, MaterialParamType::Diffuse, MaterialParamType::Specular, MaterialParamType::Shininess }, {} }
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

Graphics::MeshManager& ResourceManager::GetMeshManager() {
    return Graphics::MeshManager::GetInstance();
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