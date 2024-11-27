#include "Quad.h"

Quad::Quad() {
    // Initialize positions as std::vector<glm::vec2>
    positions = std::vector<glm::vec2>{
        { -1.0f,  1.0f },
        { -1.0f, -1.0f },
        {  1.0f, -1.0f },
        { -1.0f,  1.0f },
        {  1.0f, -1.0f },
        {  1.0f,  1.0f }
    };

    // Initialize UVs for Albedo texture
    uvs[TextureType::Albedo] = {
        { 0.0f, 1.0f },
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 0.0f, 1.0f },
        { 1.0f, 0.0f },
        { 1.0f, 1.0f }
    };

    // Initialize indices for quad
    indices = {
        0, 1, 2,
        3, 4, 5
    };

    if (lods.empty()) {
        MeshLOD defaultLOD;
        defaultLOD.indexOffset = 0;
        defaultLOD.indexCount = static_cast<uint32_t>(indices.size());
        lods.push_back(defaultLOD);
    }

    // For 2D meshes, normals, tangents, bitangents might not be necessary
}