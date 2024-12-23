#pragma once

#include <variant>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include <cfloat>
#include "Graphics/Textures/TextureType.h"

/**
 * Represents a sub-range of indices for a specific LOD level.
 */
struct MeshLOD {
    uint32_t indexOffset;  // Offset into the main "indices" vector
    uint32_t indexCount;   // Number of indices for this LOD
};

/**
 * The Mesh class holds geometry data (positions, normals, tangents, UVs, etc.)
 * for a single piece of geometry. It can contain multiple LOD levels,
 * each referencing a sub-range of the 'indices' array via MeshLOD.
 */
struct Mesh
{
    // Positions can be either vec2 or vec3
    using PositionsType = std::variant<std::vector<glm::vec2>, std::vector<glm::vec3>>;
    PositionsType positions;

    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> tangents;
    std::vector<glm::vec3> bitangents;

    // UV sets, keyed by TextureType (Albedo, Normal, etc.)
    std::unordered_map<TextureType, std::vector<glm::vec2>> uvs;

    // Consolidated index buffer + LOD sub-ranges
    std::vector<uint32_t> indices;
    std::vector<MeshLOD> lods;

    // Basic bounding volume info
    glm::vec3 localCenter = glm::vec3(0.0f);
    float boundingSphereRadius = 1.0f;
    glm::vec3 minBounds = glm::vec3(FLT_MAX);
    glm::vec3 maxBounds = glm::vec3(-FLT_MAX);

    size_t GetLODCount() const { return lods.size(); }
};