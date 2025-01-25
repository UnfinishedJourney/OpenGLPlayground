#pragma once

#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include <cfloat>
#include "Graphics/Materials/MaterialParamType.h"

struct MeshLOD {
    uint32_t indexOffset;  // Offset into the main "indices" vector
    uint32_t indexCount;   // Number of indices for this LOD
};

struct Mesh
{
    std::vector<glm::vec3> positions;

    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> tangents;
    std::vector<glm::vec3> bitangents;

    std::unordered_map<TextureType, std::vector<glm::vec2>> uvs;

    std::vector<uint32_t> indices;
    std::vector<MeshLOD> lods;

    // Basic bounding volume info
    glm::vec3 localCenter       = glm::vec3(0.0f);
    glm::vec3 minBounds         = glm::vec3(FLT_MAX);
    glm::vec3 maxBounds         = glm::vec3(-FLT_MAX);
    float boundingSphereRadius = 1.0f;

    size_t GetLODCount() const { return lods.size(); }
};