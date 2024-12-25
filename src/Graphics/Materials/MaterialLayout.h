#pragma once
#include <unordered_set>
#include "MaterialParamType.h"
#include <functional>
#include <cstdint>

struct MaterialLayout {
    std::unordered_set<MaterialParamType> params;
    std::unordered_set<TextureType> textures;

    MaterialLayout() = default;

    MaterialLayout(std::initializer_list<MaterialParamType> initParams,
        std::initializer_list<TextureType> initTextures = {})
        : params(initParams), textures(initTextures)
    {}

    bool operator==(const MaterialLayout& other) const {
        return (params == other.params) && (textures == other.textures);
    }
};

namespace std {
    template <>
    struct hash<MaterialLayout> {
        std::size_t operator()(const MaterialLayout& layout) const noexcept {
            size_t seed = 0;
            auto hash_combine = [](size_t& s, size_t v) {
                s ^= v + 0x9e3779b9 + (s << 6) + (s >> 2);
                };
            for (const auto& param : layout.params) {
                hash_combine(seed, std::hash<MaterialParamType>{}(param));
            }
            for (const auto& tex : layout.textures) {
                hash_combine(seed, std::hash<TextureType>{}(tex));
            }
            return seed;
        }
    };
}