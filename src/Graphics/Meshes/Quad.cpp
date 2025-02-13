#include "Quad.h"
#include <glm/glm.hpp>

namespace graphics {

    Quad::Quad() {
        // Define 4 vertices.
        positions_ = {
            { -1.0f,  1.0f, 0.0f },  // top-left
            { -1.0f, -1.0f, 0.0f },  // bottom-left
            {  1.0f, -1.0f, 0.0f },  // bottom-right
            {  1.0f,  1.0f, 0.0f }   // top-right
        };

        // Provide UVs for Albedo.
        uvs_[TextureType::Albedo] = {
            { 0.0f, 1.0f },
            { 0.0f, 0.0f },
            { 1.0f, 0.0f },
            { 1.0f, 1.0f }
        };

        // Two triangles (6 indices).
        indices_ = { 0, 1, 2, 0, 2, 3 };

        // Normals: all pointing in the +Z direction.
        normals_ = {
            { 0.0f, 0.0f, 1.0f },
            { 0.0f, 0.0f, 1.0f },
            { 0.0f, 0.0f, 1.0f },
            { 0.0f, 0.0f, 1.0f }
        };

        // Basic bounds.
        minBounds_ = { -1.0f, -1.0f, 0.0f };
        maxBounds_ = { 1.0f,  1.0f, 0.0f };
        localCenter_ = 0.5f * (minBounds_ + maxBounds_);
        boundingSphereRadius_ = glm::length(maxBounds_ - localCenter_);

        // Single LOD.
        MeshLOD lod0;
        lod0.indexOffset_ = 0;
        lod0.indexCount_ = static_cast<uint32_t>(indices_.size());
        lods_.push_back(lod0);
    }

    Floor::Floor() {
        // Define 4 vertices; the floor is translated and scaled.
        positions_ = {
            { -1.0f, -0.5f,  1.0f },  // top-left
            { -1.0f, -0.5f, -1.0f },  // bottom-left
            {  1.0f, -0.5f, -1.0f },  // bottom-right
            {  1.0f, -0.5f,  1.0f }   // top-right
        };

        // Scale X and Z by 10.
        for (auto& p : positions_) {
            p.x *= 10.0f;
            p.z *= 10.0f;
        }

        // Define UVs.
        uvs_[TextureType::Albedo] = {
            { 0.0f, 1.0f },
            { 0.0f, 0.0f },
            { 1.0f, 0.0f },
            { 1.0f, 1.0f }
        };

        // Two triangles.
        indices_ = { 0, 1, 2, 0, 2, 3 };

        // Normals: all pointing upward (+Y).
        normals_ = {
            { 0.0f, 1.0f, 0.0f },
            { 0.0f, 1.0f, 0.0f },
            { 0.0f, 1.0f, 0.0f },
            { 0.0f, 1.0f, 0.0f }
        };

        // Basic bounds.
        minBounds_ = { -1.0f, -1.0f, 0.0f };
        maxBounds_ = { 1.0f,  1.0f, 0.0f };
        localCenter_ = 0.5f * (minBounds_ + maxBounds_);
        boundingSphereRadius_ = glm::length(maxBounds_ - localCenter_);

        // Single LOD.
        MeshLOD lod0;
        lod0.indexOffset_ = 0;
        lod0.indexCount_ = static_cast<uint32_t>(indices_.size());
        lods_.push_back(lod0);
    }

} // namespace graphics