#pragma once
#include <unordered_set>
#include "MaterialParamType.h"

struct MaterialLayout {
    std::unordered_set<MaterialParamType> params_;
    std::unordered_set<TextureType> textures_;

    MaterialLayout() = default;
    MaterialLayout(std::initializer_list<MaterialParamType> initParams,
        std::initializer_list<TextureType> initTextures = {})
        : params_(initParams), textures_(initTextures) {}

    bool operator==(const MaterialLayout& other) const {
        return params_ == other.params_ && textures_ == other.textures_;
    }
};

namespace std {
    template <>
    struct hash<MaterialLayout> {
        std::size_t operator()(const MaterialLayout& layout) const noexcept {
            size_t seed = 0;
            auto hash_combine = [&seed](size_t value) {
                seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
                };
            for (auto p : layout.params_) {
                hash_combine(std::hash<MaterialParamType>()(p));
            }
            for (auto t : layout.textures_) {
                hash_combine(std::hash<TextureType>()(t));
            }
            return seed;
        }
    };
}