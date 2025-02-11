#include "Cube.h"
#include <glm/glm.hpp>
#include <algorithm>  // For std::min, std::max

namespace Graphics {

    Cube::Cube() {
        // 8 cube corners.
        positions_ = {
            { -0.5f, -0.5f, -0.5f },
            {  0.5f, -0.5f, -0.5f },
            {  0.5f,  0.5f, -0.5f },
            { -0.5f,  0.5f, -0.5f },
            { -0.5f, -0.5f,  0.5f },
            {  0.5f, -0.5f,  0.5f },
            {  0.5f,  0.5f,  0.5f },
            { -0.5f,  0.5f,  0.5f }
        };

        // Provide UVs for one texture type (Albedo).
        uvs_[TextureType::Albedo] = {
            { 0.0f, 0.0f },
            { 1.0f, 0.0f },
            { 1.0f, 1.0f },
            { 0.0f, 1.0f },
            { 0.0f, 0.0f },
            { 1.0f, 0.0f },
            { 1.0f, 1.0f },
            { 0.0f, 1.0f }
        };

        // 12 triangles => 36 indices.
        indices_ = {
            // Front face
            0, 1, 2, 2, 3, 0,
            // Back face
            4, 5, 6, 6, 7, 4,
            // Left face
            0, 3, 7, 7, 4, 0,
            // Right face
            1, 5, 6, 6, 2, 1,
            // Bottom face
            0, 4, 5, 5, 1, 0,
            // Top face
            3, 2, 6, 6, 7, 3
        };

        // Calculate normals per vertex by averaging face normals.
        normals_.resize(positions_.size(), glm::vec3(0.0f));
        for (size_t i = 0; i < indices_.size(); i += 3) {
            uint32_t i0 = indices_[i];
            uint32_t i1 = indices_[i + 1];
            uint32_t i2 = indices_[i + 2];
            glm::vec3 v0 = positions_[i0];
            glm::vec3 v1 = positions_[i1];
            glm::vec3 v2 = positions_[i2];
            glm::vec3 faceNormal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
            normals_[i0] += faceNormal;
            normals_[i1] += faceNormal;
            normals_[i2] += faceNormal;
        }
        for (auto& n : normals_) {
            n = glm::normalize(n);
        }

        // Set bounding volumes.
        minBounds_ = { -0.5f, -0.5f, -0.5f };
        maxBounds_ = { 0.5f,  0.5f,  0.5f };
        localCenter_ = 0.5f * (minBounds_ + maxBounds_);
        boundingSphereRadius_ = glm::length(maxBounds_ - localCenter_);

        // Single LOD covering all indices.
        MeshLOD lod0;
        lod0.indexOffset_ = 0;
        lod0.indexCount_ = static_cast<uint32_t>(indices_.size());
        lods_.push_back(lod0);
    }

} // namespace Graphics