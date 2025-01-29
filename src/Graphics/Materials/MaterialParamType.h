#pragma once

#include <cstddef>
#include <functional>

/**
 * @brief Enumerate standard parameter types for a material (like MTL).
 */
enum class MaterialParamType : std::size_t {
    Ambient = 0, // Ka
    Diffuse = 1, // Kd
    Specular = 2, // Ks
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

/**
 * @brief Enumerate texture types for the material (e.g. albedo, normal).
 *        In code, these can be used as bits in a usage mask.
 */
enum class TextureType : std::size_t {
    Albedo = 0,
    Normal = 1,
    MetalRoughness = 2,
    AO = 3,
    Emissive = 4,
    Ambient = 5,
    Height = 6,
    BRDFLut = 7,
    Unknown = 8,

    COUNT = 9
};

namespace std {
    template <>
    struct hash<TextureType> {
        std::size_t operator()(const TextureType& type) const noexcept {
            return std::hash<std::size_t>()(static_cast<std::size_t>(type));
        }
    };
}