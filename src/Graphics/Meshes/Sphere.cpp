#include "Sphere.h"
#include <stdexcept>
#include <cmath>
#include <glm/glm.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace graphics {

    Sphere::Sphere(float radius, unsigned int sectorCount, unsigned int stackCount) {
        if (radius <= 0.f)
            throw std::invalid_argument("Sphere radius must be > 0");
        if (sectorCount < 3 || stackCount < 2)
            throw std::invalid_argument("sectorCount must be >= 3 and stackCount >= 2");

        // Reserve space for vertices and UVs.
        positions_.reserve((stackCount + 1) * (sectorCount + 1));
        normals_.reserve((stackCount + 1) * (sectorCount + 1));
        uvs_[TextureType::Albedo].reserve((stackCount + 1) * (sectorCount + 1));
        indices_.reserve(stackCount * sectorCount * 6);

        float sectorStep = 2.f * static_cast<float>(M_PI) / sectorCount;
        float stackStep = static_cast<float>(M_PI) / stackCount;
        float invRadius = 1.f / radius;

        // Generate vertices.
        for (unsigned int i = 0; i <= stackCount; ++i) {
            float stackAngle = static_cast<float>(M_PI) / 2.f - i * stackStep;
            float xy = radius * std::cos(stackAngle);
            float z = radius * std::sin(stackAngle);
            for (unsigned int j = 0; j <= sectorCount; ++j) {
                float sectorAngle = j * sectorStep;
                float x = xy * std::cos(sectorAngle);
                float y = xy * std::sin(sectorAngle);
                positions_.push_back({ x, y, z });
                glm::vec3 n = glm::normalize(glm::vec3(x, y, z));
                normals_.push_back(n);
                float s = static_cast<float>(j) / sectorCount;
                float t = static_cast<float>(i) / stackCount;
                uvs_[TextureType::Albedo].push_back({ s, t });
            }
        }

        // Generate indices.
        for (unsigned int i = 0; i < stackCount; ++i) {
            unsigned int k1 = i * (sectorCount + 1);
            unsigned int k2 = k1 + sectorCount + 1;
            for (unsigned int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
                if (i != 0) {
                    indices_.push_back(k1);
                    indices_.push_back(k2);
                    indices_.push_back(k1 + 1);
                }
                if (i != (stackCount - 1)) {
                    indices_.push_back(k1 + 1);
                    indices_.push_back(k2);
                    indices_.push_back(k2 + 1);
                }
            }
        }

        // Set bounding volumes.
        minBounds_ = { -radius, -radius, -radius };
        maxBounds_ = { radius,  radius,  radius };
        localCenter_ = glm::vec3(0.f);
        boundingSphereRadius_ = radius;

        // Single LOD.
        if (lods_.empty()) {
            MeshLOD lod0;
            lod0.indexOffset_ = 0;
            lod0.indexCount_ = static_cast<uint32_t>(indices_.size());
            lods_.push_back(lod0);
        }
    }

} // namespace graphics