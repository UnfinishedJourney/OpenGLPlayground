#include "Quad.h"

Quad::Quad() {
    positions = {
        { -1.0f,  1.0f, 0.0f }, // Top-left
        { -1.0f, -1.0f, 0.0f }, // Bottom-left
        {  1.0f, -1.0f, 0.0f }, // Bottom-right
        {  1.0f,  1.0f, 0.0f }  // Top-right
    };

    uvs[TextureType::Albedo] = {
        { 0.0f, 1.0f }, // Top-left
        { 0.0f, 0.0f }, // Bottom-left
        { 1.0f, 0.0f }, // Bottom-right
        { 1.0f, 1.0f }  // Top-right
    };

    indices = {
        0, 1, 2, // First triangle
        0, 2, 3  // Second triangle
    };

    if (lods.empty()) {
        MeshLOD defaultLOD;
        defaultLOD.indexOffset = 0;
        defaultLOD.indexCount = static_cast<uint32_t>(indices.size());
        lods.push_back(defaultLOD);
    }

    normals.reserve(4);
    for (int i = 0; i < 4; ++i) {
        normals.emplace_back(0.0f, 0.0f, 1.0f); // Assuming quad faces +Z
    }

}