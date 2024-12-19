#pragma once
#include <unordered_set>

enum class TextureType; // forward

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

namespace std {
    template <>
    struct hash<MeshLayout> {
        std::size_t operator()(const MeshLayout& layout) const noexcept {
            size_t seed = 0;
            auto hash_combine = [](size_t& s, size_t v) {
                s ^= v + 0x9e3779b9 + (s << 6) + (s >> 2);
                };
            hash_combine(seed, std::hash<bool>{}(layout.hasPositions));
            hash_combine(seed, std::hash<bool>{}(layout.hasNormals));
            hash_combine(seed, std::hash<bool>{}(layout.hasTangents));
            hash_combine(seed, std::hash<bool>{}(layout.hasBitangents));
            for (auto t : layout.textureTypes) {
                hash_combine(seed, std::hash<int>{}((int)t));
            }
            return seed;
        }
    };
}