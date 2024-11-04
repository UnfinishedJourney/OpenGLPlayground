#pragma once

#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include "Utilities/Utility.h"
struct Mesh {
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> tangents;
    std::vector<glm::vec3> bitangents;
    std::unordered_map<TextureType, std::vector<glm::vec2>> uvs;
    std::vector<uint32_t> indices;

    // Optional utility methods
    size_t GetVertexCount() const { return positions.size(); }
    size_t GetIndexCount() const { return indices.size(); }

    bool HasNormals() const { return !normals.empty(); }
    bool HasTangents() const { return !tangents.empty(); }
    bool HasBitangents() const { return !bitangents.empty(); }
    bool HasTextureCoords(TextureType type) const {
        auto it = uvs.find(type);
        return it != uvs.end() && !it->second.empty();
    }
};