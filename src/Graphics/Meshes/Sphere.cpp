#include "Graphics/Meshes/Sphere.h"
#include <cmath>

// Define PI if not already defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Sphere::Sphere(float radius, unsigned int sectorCount, unsigned int stackCount)
{
    // Clear any existing data
    positions.clear();
    normals.clear();
    uvs.clear();
    indices.clear();

    // Calculate step sizes
    float x, y, z, xy;                              // vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
    float s, t;                                     // vertex texCoord

    float sectorStep = 2 * M_PI / sectorCount;
    float stackStep = M_PI / stackCount;
    float sectorAngle, stackAngle;

    // Generate vertices
    for (unsigned int i = 0; i <= stackCount; ++i)
    {
        stackAngle = M_PI / 2 - i * stackStep;        // from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);             // r * cos(u)
        z = radius * sinf(stackAngle);              // r * sin(u)

        // Add (sectorCount + 1) vertices per stack
        for (unsigned int j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = j * sectorStep;           // from 0 to 2pi

            // Vertex position
            x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
            positions.emplace_back(glm::vec3(x, y, z));

            // Normalized vertex normal
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            normals.emplace_back(glm::vec3(nx, ny, nz));

            // Vertex texture coordinate (s, t) range between [0, 1]
            s = (float)j / sectorCount;
            t = (float)i / stackCount;
            uvs[TextureType::Albedo].emplace_back(glm::vec2(s, t));
        }
    }

    // Generate indices
    unsigned int k1, k2;
    for (unsigned int i = 0; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1);     // beginning of current stack
        k2 = k1 + sectorCount + 1;      // beginning of next stack

        for (unsigned int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            // Skip first stack
            if (i != 0)
            {
                indices.emplace_back(k1);
                indices.emplace_back(k2);
                indices.emplace_back(k1 + 1);
            }

            // Skip last stack
            if (i != (stackCount - 1))
            {
                indices.emplace_back(k1 + 1);
                indices.emplace_back(k2);
                indices.emplace_back(k2 + 1);
            }
        }
    }
}