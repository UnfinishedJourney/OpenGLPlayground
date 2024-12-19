#pragma once
#include <unordered_set>
#include "MaterialParamType.h"
#include "Graphics/Textures/TextureType.h"

struct MaterialLayout {
    std::unordered_set<MaterialParamType> params;
    std::unordered_set<TextureType> textures;

    bool operator==(const MaterialLayout& other) const {
        return params == other.params && textures == other.textures;
    }
};