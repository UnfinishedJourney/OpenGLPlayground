#include "Quad.h"
#include <glm/glm.hpp>

Quad::Quad()
{
    // 4 corners, spanning -1..1 in XY
    positions = {
        { -1.0f,  1.0f, 0.0f }, // top-left
        { -1.0f, -1.0f, 0.0f }, // bottom-left
        {  1.0f, -1.0f, 0.0f }, // bottom-right
        {  1.0f,  1.0f, 0.0f }  // top-right
    };

    // We'll define albedo UVs
    uvs[TextureType::Albedo] = {
        { 0.0f, 1.0f }, // top-left
        { 0.0f, 0.0f }, // bottom-left
        { 1.0f, 0.0f }, // bottom-right
        { 1.0f, 1.0f }  // top-right
    };

    // Two triangles => 6 indices
    indices = {
        0, 1, 2,
        0, 2, 3
    };

    // Normals: +Z for all
    normals = {
        { 0.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 1.0f }
    };

    // Basic bounds
    minBounds = { -1.0f, -1.0f, 0.0f };
    maxBounds = { 1.0f,  1.0f, 0.0f };
    localCenter = 0.5f * (minBounds + maxBounds);
    boundingSphereRadius = glm::length(maxBounds - localCenter);

    // Single LOD
    MeshLOD lod0;
    lod0.indexOffset = 0;
    lod0.indexCount = static_cast<uint32_t>(indices.size());
    lods.push_back(lod0);
}