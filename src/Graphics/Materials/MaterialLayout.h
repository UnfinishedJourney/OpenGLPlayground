#pragma once
#include <bitset>
#include <initializer_list>
#include "MaterialParamType.h"

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
            unsigned long paramsValue = layout.params_.to_ulong();
            unsigned long texturesValue = layout.textures_.to_ulong();
            return (paramsValue << kTextureTypeCount) | texturesValue;
        }
    };
}