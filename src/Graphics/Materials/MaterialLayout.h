#pragma once
#include <unordered_set>
#include "MaterialParamType.h"
#include "Graphics/Textures/TextureType.h"

/**
 * MaterialLayout stores which parameters and textures are actually used
 * in this material, so we can skip uploading unused ones.
 */
struct MaterialLayout {
    std::unordered_set<MaterialParamType> params;
    std::unordered_set<TextureType> textures;

    bool operator==(const MaterialLayout& other) const {
        return (params == other.params) && (textures == other.textures);
    }
};