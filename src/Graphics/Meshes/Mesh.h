#pragma once

#include <vector>
#include <unordered_map>
#include <variant>
#include <glm/glm.hpp>
#include "Utilities/Utility.h"

struct Mesh {
    using PositionsType = std::variant<std::vector<glm::vec2>, std::vector<glm::vec3>>;

    PositionsType positions; // Positions can be either vec2 or vec3
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> tangents;
    std::vector<glm::vec3> bitangents;
    std::unordered_map<TextureType, std::vector<glm::vec2>> uvs;
    std::vector<uint32_t> indices;

    // Optional utility methods
    size_t GetVertexCount() const {
        return std::visit([](auto&& arg) -> size_t { return arg.size(); }, positions);
    }

    size_t GetIndexCount() const { return indices.size(); }

    bool HasNormals() const { return !normals.empty(); }
    bool HasTangents() const { return !tangents.empty(); }
    bool HasBitangents() const { return !bitangents.empty(); }
    bool HasTextureCoords(TextureType type) const {
        auto it = uvs.find(type);
        return it != uvs.end() && !it->second.empty();
    }
};