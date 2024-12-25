#include "Sphere.h"
#include <cmath>
#include <stdexcept>

// Define M_PI if not defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Sphere::Sphere(float radius, unsigned int sectorCount, unsigned int stackCount)
{
    // Validate inputs
    if (radius <= 0.0f) {
        throw std::invalid_argument("Radius must be positive.");
    }
    if (sectorCount < 3 || stackCount < 2) {
        throw std::invalid_argument("Sector count must be >= 3 and stack count must be >= 2.");
    }

    // Initialize positions as std::vector<glm::vec3>
    positions = std::vector<glm::vec3>();
    auto& positionsVec = positions;
    positionsVec.reserve((stackCount + 1) * (sectorCount + 1));

    if (sectorCount == 0 || stackCount == 0) {
        throw std::invalid_argument("Sector and stack counts must be greater than 0.");
    }

    // Initialize normals as std::vector<glm::vec3>
    normals.reserve((stackCount + 1) * (sectorCount + 1));

    // Initialize UVs for Albedo texture
    uvs[TextureType::Albedo].reserve((stackCount + 1) * (sectorCount + 1));

    // Initialize indices
    indices.reserve(stackCount * sectorCount * 6);

    float x, y, z, xy;
    float nx, ny, nz, lengthInv = 1.0f / radius;
    float s, t;

    float sectorStep = 2 * static_cast<float>(M_PI) / sectorCount;
    float stackStep = static_cast<float>(M_PI) / stackCount;
    float sectorAngle, stackAngle;

    // Generate vertices
    for (unsigned int i = 0; i <= stackCount; ++i) {
        stackAngle = static_cast<float>(M_PI) / 2 - i * stackStep; // from pi/2 to -pi/2
        xy = radius * cosf(stackAngle); // r * cos(u)
        z = radius * sinf(stackAngle);  // r * sin(u)

        // Add (sectorCount+1) vertices per stack
        for (unsigned int j = 0; j <= sectorCount; ++j) {
            sectorAngle = j * sectorStep; // from 0 to 2pi

            // Vertex position
            x = xy * cosf(sectorAngle); // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle); // r * cos(u) * sin(v)
            positionsVec.emplace_back(x, y, z);

            // Normalized vertex normal
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            normals.emplace_back(nx, ny, nz);

            // Vertex UV coordinates
            s = static_cast<float>(j) / sectorCount;
            t = static_cast<float>(i) / stackCount;
            uvs[TextureType::Albedo].emplace_back(s, t);
        }
    }

    // Generate indices
    unsigned int k1, k2;
    for (unsigned int i = 0; i < stackCount; ++i) {
        k1 = i * (sectorCount + 1);     // beginning of current stack
        k2 = k1 + sectorCount + 1;      // beginning of next stack

        for (unsigned int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            // 2 triangles per sector excluding first and last stacks
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (stackCount - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    if (lods.empty()) {
        MeshLOD defaultLOD;
        defaultLOD.indexOffset = 0;
        defaultLOD.indexCount = static_cast<uint32_t>(indices.size());
        lods.push_back(defaultLOD);
    }
}