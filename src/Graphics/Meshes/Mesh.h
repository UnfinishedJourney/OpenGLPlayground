#pragma once

#include <vector>
#include <unordered_map>
#include <cfloat>         // for FLT_MAX
#include <glm/glm.hpp>

#include "Graphics/Materials/MaterialParamType.h" // For TextureType
#include "Graphics/Meshes/MeshLayout.h"

/**
 * @brief A sub-structure holding index offset/count for a particular LOD.
 */
struct MeshLOD {
    uint32_t indexOffset = 0;
    uint32_t indexCount = 0;
};

/**
 * @brief The core geometric data for a single mesh:
 *        Positions, normals, tangents, bitangents, uv sets, indices, LOD info, bounding volumes.
 */
struct Mesh
{
    // Vertex data
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> tangents;
    std::vector<glm::vec3> bitangents;

    // UV sets keyed by TextureType (often just one set, but you can store multiple)
    std::unordered_map<TextureType, std::vector<glm::vec2>> uvs;

    // Index data & LOD info
    std::vector<uint32_t> indices;
    std::vector<MeshLOD>  lods;

    // Basic bounding volume
    glm::vec3 minBounds = glm::vec3(FLT_MAX);
    glm::vec3 maxBounds = glm::vec3(-FLT_MAX);
    glm::vec3 localCenter = glm::vec3(0.0f);
    float     boundingSphereRadius = 1.0f;

    // Utility
    size_t GetLODCount() const { return lods.size(); }
};