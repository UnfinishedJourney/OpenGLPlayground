#include "Cube.h"
#include <glm/glm.hpp>
#include <algorithm> // for std::max, etc.

/**
 * @brief Constructs a cube of side length=1, centered at (0,0,0).
 *        Each face has 2 triangles, total 36 indices, 8 vertices.
 */
Cube::Cube()
{
    // 8 corners
    positions = {
        { -0.5f, -0.5f, -0.5f },
        {  0.5f, -0.5f, -0.5f },
        {  0.5f,  0.5f, -0.5f },
        { -0.5f,  0.5f, -0.5f },
        { -0.5f, -0.5f,  0.5f },
        {  0.5f, -0.5f,  0.5f },
        {  0.5f,  0.5f,  0.5f },
        { -0.5f,  0.5f,  0.5f }
    };

    // We'll provide at least Albedo UVs
    uvs[TextureType::Albedo] = {
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 1.0f, 1.0f },
        { 0.0f, 1.0f },
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 1.0f, 1.0f },
        { 0.0f, 1.0f }
    };

    // 12 triangles => 36 indices
    indices = {
        // Front face
        0, 1, 2,
        2, 3, 0,

        // Back face
        4, 5, 6,
        6, 7, 4,

        // Left face
        0, 3, 7,
        7, 4, 0,

        // Right face
        1, 5, 6,
        6, 2, 1,

        // Bottom face
        0, 4, 5,
        5, 1, 0,

        // Top face
        3, 2, 6,
        6, 7, 3
    };

    // Calculate normals
    normals.resize(positions.size(), glm::vec3(0.0f));
    for (size_t i = 0; i < indices.size(); i += 3) {
        uint32_t i0 = indices[i + 0];
        uint32_t i1 = indices[i + 1];
        uint32_t i2 = indices[i + 2];

        glm::vec3 v0 = positions[i0];
        glm::vec3 v1 = positions[i1];
        glm::vec3 v2 = positions[i2];

        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

        normals[i0] += faceNormal;
        normals[i1] += faceNormal;
        normals[i2] += faceNormal;
    }
    for (auto& n : normals) {
        n = glm::normalize(n);
    }

    // Basic bounding box
    minBounds = { -0.5f, -0.5f, -0.5f };
    maxBounds = { 0.5f,  0.5f,  0.5f };
    localCenter = 0.5f * (minBounds + maxBounds);
    boundingSphereRadius = glm::length(maxBounds - localCenter);

    // Single LOD
    MeshLOD lod0;
    lod0.indexOffset = 0;
    lod0.indexCount = static_cast<uint32_t>(indices.size());
    lods.push_back(lod0);
}