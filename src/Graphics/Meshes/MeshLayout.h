#pragma once

#include <unordered_set>

enum class TextureType; // Forward declaration

struct MeshLayout {
    bool hasPositions = true;
    bool hasNormals = false;
    bool hasTangents = false;
    bool hasBitangents = false;
    std::unordered_set<TextureType> textureTypes;

    bool operator==(const MeshLayout& other) const {
        return hasPositions == other.hasPositions &&
            hasNormals == other.hasNormals &&
            hasTangents == other.hasTangents &&
            hasBitangents == other.hasBitangents &&
            textureTypes == other.textureTypes;
    }
};

// Hash function for MeshLayout
namespace std {
    template <>
    struct hash<MeshLayout> {
        std::size_t operator()(const MeshLayout& layout) const noexcept {
            size_t seed = 0;
            auto hash_combine = [](size_t& seed, size_t value) {
                seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
                };

            hash_combine(seed, std::hash<bool>{}(layout.hasPositions));
            hash_combine(seed, std::hash<bool>{}(layout.hasNormals));
            hash_combine(seed, std::hash<bool>{}(layout.hasTangents));
            hash_combine(seed, std::hash<bool>{}(layout.hasBitangents));
            for (const auto& texType : layout.textureTypes) {
                hash_combine(seed, std::hash<int>{}(static_cast<int>(texType)));
            }
            return seed;
        }
    };
}