#pragma once

#include <bitset>
#include <cstddef>
#include "Graphics/Materials/MaterialParamType.h"  // Must define TextureType and TextureType::COUNT

/**
 * @brief Describes which vertex attributes to load for a mesh.
 */
struct MeshLayout {
    bool hasPositions_ = true;
    bool hasNormals_ = false;
    bool hasTangents_ = false;
    bool hasBitangents_ = false;
    std::bitset<kTextureTypeCount> textureTypes_;  // Does not accept an initializer list directly

    // Default constructor
    MeshLayout() = default;

    // Custom constructor that accepts an initializer list for texture types.
    MeshLayout(bool positions, bool normals, bool tangents, bool bitangents, std::initializer_list<TextureType> texTypes)
        : hasPositions_(positions)
        , hasNormals_(normals)
        , hasTangents_(tangents)
        , hasBitangents_(bitangents)
    {
        for (auto tex : texTypes) {
            textureTypes_.set(static_cast<std::size_t>(tex), true);
        }
    }

    bool operator==(const MeshLayout& other) const {
        return hasPositions_ == other.hasPositions_ &&
            hasNormals_ == other.hasNormals_ &&
            hasTangents_ == other.hasTangents_ &&
            hasBitangents_ == other.hasBitangents_ &&
            textureTypes_ == other.textureTypes_;
    }
};

namespace std {
    template <>
    struct hash<MeshLayout> {
        size_t operator()(const MeshLayout& layout) const noexcept {
            size_t seed = 0;
            auto hash_combine = [&seed](size_t value) {
                seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
                };
            hash_combine(std::hash<bool>{}(layout.hasPositions_));
            hash_combine(std::hash<bool>{}(layout.hasNormals_));
            hash_combine(std::hash<bool>{}(layout.hasTangents_));
            hash_combine(std::hash<bool>{}(layout.hasBitangents_));
            hash_combine(std::hash<std::string>{}(layout.textureTypes_.to_string()));
            return seed;
        }
    };
}