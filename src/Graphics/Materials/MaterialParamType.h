#pragma once
#include <cstddef>

enum class MaterialParamType : std::size_t {
    Ambient = 0,   // Ka
    Diffuse = 1,   // Kd
    Specular = 2,  // Ks
    Shininess = 3, // Ns
    COUNT = 4
};

namespace std {
    template <>
    struct hash<MaterialParamType> {
        std::size_t operator()(const MaterialParamType& type) const noexcept {
            return std::hash<std::size_t>()(static_cast<std::size_t>(type));
        }
    };
}

//maybe should move it to separate class
enum class TextureType : std::size_t {
    Albedo = 0,
    Normal = 1,
    MetalRoughness = 2,
    AO = 3,
    Emissive = 4,
    Ambient = 5,
    Height = 6,
    BRDFLut = 7,
    SkyBox = 8,
    SkyboxIrradiance = 9,
    ShadowMap = 10,
    Unknown = 11,
    COUNT = 12
};

namespace std {
    template <>
    struct hash<TextureType> {
        std::size_t operator()(const TextureType& type) const noexcept {
            return std::hash<std::size_t>()(static_cast<std::size_t>(type));
        }
    };
}