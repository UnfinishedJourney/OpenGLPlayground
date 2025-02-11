#pragma once
#include "MaterialParamType.h"
#include <bitset>
#include <initializer_list>

constexpr std::size_t kMaterialParamCount = static_cast<std::size_t>(MaterialParamType::COUNT);
constexpr std::size_t kTextureTypeCount = static_cast<std::size_t>(TextureType::COUNT);

struct MaterialLayout {
    std::bitset<kMaterialParamCount> params_;
    std::bitset<kTextureTypeCount> textures_;

    MaterialLayout() = default;

    MaterialLayout(std::initializer_list<MaterialParamType> initParams,
        std::initializer_list<TextureType> initTextures = {}) {
        for (auto p : initParams) {
            params_.set(static_cast<std::size_t>(p), true);
        }
        for (auto t : initTextures) {
            textures_.set(static_cast<std::size_t>(t), true);
        }
    }

    bool HasParam(MaterialParamType param) const {
        return params_.test(static_cast<std::size_t>(param));
    }

    bool HasTexture(TextureType tex) const {
        return textures_.test(static_cast<std::size_t>(tex));
    }

    bool operator==(const MaterialLayout& other) const {
        return params_ == other.params_ && textures_ == other.textures_;
    }
};

namespace std {
    template <>
    struct hash<MaterialLayout> {
        std::size_t operator()(const MaterialLayout& layout) const noexcept {
            // Convert each bitset to an unsigned long.
            // (This works safely because kMaterialParamCount <= sizeof(unsigned long)*8, etc.)
            unsigned long paramsValue = layout.params_.to_ulong();
            unsigned long texturesValue = layout.textures_.to_ulong();
            // Shift the params bits left by the number of texture bits and then OR with textures.
            return (paramsValue << kTextureTypeCount) | texturesValue;
        }
    };
}