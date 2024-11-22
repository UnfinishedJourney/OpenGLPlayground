#include "Cube.h"

// Optional: Include additional headers if needed
#include <stdexcept>

Cube::Cube() {
    // Initialize positions as std::vector<glm::vec3>
    positions = std::vector<glm::vec3>{
        { -0.5f, -0.5f, -0.5f },
        {  0.5f, -0.5f, -0.5f },
        {  0.5f,  0.5f, -0.5f },
        { -0.5f,  0.5f, -0.5f },
        { -0.5f, -0.5f,  0.5f },
        {  0.5f, -0.5f,  0.5f },
        {  0.5f,  0.5f,  0.5f },
        { -0.5f,  0.5f,  0.5f }
    };

    // Initialize UVs for Albedo texture
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

    // Initialize indices for cube faces
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
    normals.resize(std::get<std::vector<glm::vec3>>(positions).size(), glm::vec3(0.0f));

    for (size_t i = 0; i < indices.size(); i += 3) {
        uint32_t idx0 = indices[i];
        uint32_t idx1 = indices[i + 1];
        uint32_t idx2 = indices[i + 2];

        glm::vec3 v0 = std::get<std::vector<glm::vec3>>(positions)[idx0];
        glm::vec3 v1 = std::get<std::vector<glm::vec3>>(positions)[idx1];
        glm::vec3 v2 = std::get<std::vector<glm::vec3>>(positions)[idx2];

        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

        normals[idx0] += normal;
        normals[idx1] += normal;
        normals[idx2] += normal;
    }

    for (auto& normal : normals) {
        normal = glm::normalize(normal);
    }
}