#pragma once

#include <vector>
#include <cfloat>       // For FLT_MAX
#include <cstdint>
#include <glm/glm.hpp>
#include <unordered_map>
#include "Graphics/Materials/MaterialParamType.h"  // Assumes TextureType is defined in a header

namespace graphics {

    /// Represents one level-of-detail (LOD) for a mesh.
    struct MeshLOD {
        uint32_t indexOffset_ = 0;
        uint32_t indexCount_ = 0;
    };

    /**
     * @brief Core geometric data for a mesh.
     *
     * Contains vertex positions, normals, tangents, bitangents, UV sets,
     * index data, LOD information, and bounding volumes.
     */
    struct Mesh {
        // Vertex data.
        std::vector<glm::vec3> positions_;
        std::vector<glm::vec3> normals_;
        std::vector<glm::vec3> tangents_;

        // UV sets keyed by TextureType (e.g., Albedo).
        std::unordered_map<TextureType, std::vector<glm::vec2>> uvs_;

        // Index data and LOD info.
        std::vector<uint32_t> indices_;
        std::vector<MeshLOD>  lods_;

        // Bounding volume data.
        glm::vec3 minBounds_ = glm::vec3(FLT_MAX);
        glm::vec3 maxBounds_ = glm::vec3(-FLT_MAX);
        glm::vec3 localCenter_ = glm::vec3(0.0f);
        float     boundingSphereRadius_ = 1.0f;

        /// @return the number of LOD levels.
        size_t GetLODCount() const { return lods_.size(); }
    };

} // namespace graphics