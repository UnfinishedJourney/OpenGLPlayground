#include "Sphere.h"
#include <stdexcept>
#include <cmath>
#include <glm/glm.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Sphere::Sphere(float radius, unsigned int sectorCount, unsigned int stackCount)
{
    if (radius <= 0.f)
        throw std::invalid_argument("Sphere radius must be > 0");
    if (sectorCount < 3 || stackCount < 2)
        throw std::invalid_argument("sectorCount must >= 3, stackCount >= 2");

    // Reserve space
    positions.reserve((stackCount + 1) * (sectorCount + 1));
    normals.reserve((stackCount + 1) * (sectorCount + 1));
    uvs[TextureType::Albedo].reserve((stackCount + 1) * (sectorCount + 1));

    indices.reserve(stackCount * sectorCount * 6);

    float x, y, z, xy;
    float nx, ny, nz;
    float sectorStep = 2.f * static_cast<float>(M_PI) / sectorCount;
    float stackStep = static_cast<float>(M_PI) / stackCount;
    float sectorAngle, stackAngle;
    float invRadius = 1.f / radius;

    // Generate vertices
    for (unsigned int i = 0; i <= stackCount; i++) {
        // stackAngle goes from pi/2 to -pi/2
        stackAngle = static_cast<float>(M_PI) / 2.f - i * stackStep;
        xy = radius * std::cos(stackAngle);
        z = radius * std::sin(stackAngle);

        for (unsigned int j = 0; j <= sectorCount; j++) {
            sectorAngle = j * sectorStep;

            // Vertex position
            x = xy * std::cos(sectorAngle);
            y = xy * std::sin(sectorAngle);
            positions.push_back({ x, y, z });

            // Normal
            nx = x * invRadius;
            ny = y * invRadius;
            nz = z * invRadius;
            normals.push_back(glm::normalize(glm::vec3(nx, ny, nz)));

            // UV (0..1 range)
            float s = static_cast<float>(j) / sectorCount;
            float t = static_cast<float>(i) / stackCount;
            uvs[TextureType::Albedo].push_back({ s, t });
        }
    }

    // Generate indices
    for (unsigned int i = 0; i < stackCount; i++) {
        unsigned int k1 = i * (sectorCount + 1);
        unsigned int k2 = k1 + sectorCount + 1;

        for (unsigned int j = 0; j < sectorCount; j++, k1++, k2++) {
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

    // Basic bounds
    minBounds = { -radius, -radius, -radius };
    maxBounds = { radius,  radius,  radius };
    localCenter = glm::vec3(0.f);
    boundingSphereRadius = radius;

    // Single LOD
    if (lods.empty()) {
        MeshLOD lod0;
        lod0.indexOffset = 0;
        lod0.indexCount = static_cast<uint32_t>(indices.size());
        lods.push_back(lod0);
    }
}