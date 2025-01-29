#pragma once

#include <unordered_set>
#include "MaterialParamType.h"
#include <cstdint>

/**
 * @brief Describes which material parameters (e.g., ambient, diffuse)
 *        and which texture types (e.g., albedo, normal) a given material uses.
 *
 * This helps ensure you only set relevant params/textures.
 */
struct MaterialLayout {
    std::unordered_set<MaterialParamType> params;
    std::unordered_set<TextureType>       textures;

    MaterialLayout() = default;
    MaterialLayout(std::initializer_list<MaterialParamType> initParams,
        std::initializer_list<TextureType> initTextures = {})
        : params(initParams)
        , textures(initTextures)
    {}

    bool operator==(const MaterialLayout& other) const {
        return params == other.params && textures == other.textures;
    }
};

// For std::unordered_map<MaterialLayout, ...> usage (optional)
namespace std {
    template <>
    struct hash<MaterialLayout> {
        std::size_t operator()(const MaterialLayout& layout) const noexcept {
            size_t seed = 0;
            auto hash_combine = [](size_t& s, size_t v) {
                // typical boost::hash_combine
                s ^= v + 0x9e3779b9 + (s << 6) + (s >> 2);
                };
            for (auto p : layout.params) {
                hash_combine(seed, std::hash<MaterialParamType>{}(p));
            }
            for (auto t : layout.textures) {
                hash_combine(seed, std::hash<TextureType>{}(t));
            }
            return seed;
        }
    };
}