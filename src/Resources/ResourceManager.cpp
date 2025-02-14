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
            MeshLayout{ true, true, true, false, { TextureType::Albedo } },
            MaterialLayout{ {MaterialParamType::Ambient, MaterialParamType::Diffuse, MaterialParamType::Specular, MaterialParamType::Shininess, MaterialParamType::Opacity, MaterialParamType::Emissive }, { TextureType::Albedo, TextureType::Height, TextureType::Normal, TextureType::MetalRoughness, TextureType::AO, TextureType::Emissive, TextureType::BRDFLut, TextureType::Ambient } }
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

graphics::MeshManager& ResourceManager::GetMeshManager() {
    return graphics::MeshManager::GetInstance();
}

//ModelManager& ResourceManager::GetModelManager() {
//    return ModelManager::GetInstance();
//}

graphics::TextureManager& ResourceManager::GetTextureManager() {
    return graphics::TextureManager::GetInstance();
}

graphics::MaterialManager& ResourceManager::GetMaterialManager() {
    return graphics::MaterialManager::GetInstance();
}

graphics::ShaderManager& ResourceManager::GetShaderManager() {
    return graphics::ShaderManager::GetInstance();
}