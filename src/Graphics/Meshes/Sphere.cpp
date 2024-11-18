#include "Graphics/Meshes/Sphere.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Sphere::Sphere(float radius, unsigned int sectorCount, unsigned int stackCount)
{
    positions = std::vector<glm::vec3>{};
    auto& positionsVec = std::get<std::vector<glm::vec3>>(positions);
    positionsVec.clear();
    normals.clear();
    uvs.clear();
    indices.clear();

    // Calculate step sizes
    float x, y, z, xy;
    float nx, ny, nz, lengthInv = 1.0f / radius;
    float s, t;

    float sectorStep = 2 * M_PI / sectorCount;
    float stackStep = M_PI / stackCount;
    float sectorAngle, stackAngle;

    // Generate vertices
    for (unsigned int i = 0; i <= stackCount; ++i) {
        stackAngle = M_PI / 2 - i * stackStep;
        xy = radius * cosf(stackAngle);
        z = radius * sinf(stackAngle);

        for (unsigned int j = 0; j <= sectorCount; ++j) {
            sectorAngle = j * sectorStep;

            x = xy * cosf(sectorAngle);
            y = xy * sinf(sectorAngle);
            positionsVec.emplace_back(x, y, z);

            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            normals.emplace_back(nx, ny, nz);

            s = (float)j / sectorCount;
            t = (float)i / stackCount;
            uvs[TextureType::Albedo].emplace_back(s, t);
        }
    }

    // Generate indices
    unsigned int k1, k2;
    for (unsigned int i = 0; i < stackCount; ++i) {
        k1 = i * (sectorCount + 1);
        k2 = k1 + sectorCount + 1;

        for (unsigned int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
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
}