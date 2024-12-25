#include "TerrainMesh.h"

TerrainMesh::TerrainMesh() {
    // Initialize positions as std::vector<glm::vec3>
    positions = std::vector<glm::vec3>{
        { -1.0f, 0.0f, -1.0f },
        {  1.0f, 0.0f, -1.0f },
        {  1.0f, 0.0f,  1.0f },
        { -1.0f, 0.0f,  1.0f }
    };

    // Initialize UVs for Albedo texture
    uvs[TextureType::Albedo] = {
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 1.0f, 1.0f },
        { 0.0f, 1.0f }
    };

    // Initialize indices for terrain quad
    indices = { 0, 1, 2, 2, 3, 0 };

    // Optionally, initialize normals (all pointing up)
    normals.resize(positions.size(), glm::vec3(0.0f, 1.0f, 0.0f));

    // Tangents and bitangents can be calculated if needed
}