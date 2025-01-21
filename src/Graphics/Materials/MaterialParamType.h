#pragma once
#include <bitset>
#include <cstdint>
#include <cstddef>
#include <functional>
//constexpr std::size_t MAX_MATERIAL_PARAMS = 32;
//constexpr std::size_t MAX_TEXTURE_TYPES = 32;

enum class MaterialParamType : std::size_t {
    Ambient = 0, // Bit 0
    Diffuse = 1, // Bit 1
    Specular = 2, // Bit 2
    Shininess = 3, // Bit 3
    COUNT = 4  // Number of parameters
};

namespace std {
    template <>
    struct hash<MaterialParamType> {
        std::size_t operator()(const MaterialParamType& type) const noexcept {
            return std::hash<std::size_t>()(static_cast<std::size_t>(type));
        }
    };
}

enum class TextureType : std::size_t {
    Albedo          = 0,            // bit 0
    Normal          = 1,            // bit 1
    MetalRoughness  = 2,            // bit 2
    AO              = 3,            // bit 3
    Emissive        = 4,            // bit 4
    Ambient         = 5,
    Height          = 6,

    BRDFLut         = 7,            // NEW: dedicated slot for BRDF LUT

    Unknown         = 8,

    COUNT           = 9
};


namespace std {
    template <>
    struct hash<TextureType> {
        std::size_t operator()(const TextureType& type) const noexcept {
            return std::hash<std::size_t>()(static_cast<std::size_t>(type));
        }
    };
}