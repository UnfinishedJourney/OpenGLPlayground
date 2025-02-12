#pragma once

#include <memory>
#include <vector>
#include <string>
#include <glad/glad.h>
#include <cstddef>
#include "Renderer/RenderObject.h"  // Defines BaseRenderObject
#include "Graphics/Buffers/VertexArray.h"
#include "Graphics/Buffers/VertexBuffer.h"
#include "Graphics/Buffers/IndexBuffer.h"
#include "Graphics/Buffers/VertexBufferLayout.h"
#include "Graphics/Buffers/IndirectBuffer.h"
#include "Graphics/Meshes/MeshLayout.h"

namespace Graphics {

    /**
     * @brief Structure for one multi-draw command (used with glMultiDrawElementsIndirect).
     */
    struct DrawElementsIndirectCommand {
        GLuint count_;         ///< Number of indices to draw.
        GLuint instanceCount_; ///< Number of instances.
        GLuint firstIndex_;    ///< Starting index in the IBO.
        GLint  baseVertex_;    ///< Offset added to indices.
        GLuint baseInstance_;  ///< Base instance ID.
    };

    /**
     * @brief Info about one LOD range. Tracks the index offset and count within the combined IBO.
     */
    struct LODInfo {
        size_t indexOffsetInCombinedBuffer_ = 0;
        size_t indexCount_ = 0;
    };

    /**
     * @brief Batch groups multiple RenderObjects sharing the same shader and material.
     *
     * It merges their vertex/index data into single GPU buffers (VBO, IBO, and an indirect draw buffer)
     * and uses glMultiDrawElementsIndirect to issue a multi-draw call.
     */
    class Batch {
    public:
        Batch(const std::string& shaderName, int materialID);
        ~Batch();

        /// @brief Adds a RenderObject to the batch.
        void AddRenderObject(const std::shared_ptr<BaseRenderObject>& renderObject);

        /// @brief Returns the list of RenderObjects in the batch.
        const std::vector<std::shared_ptr<BaseRenderObject>>& GetRenderObjects() const;

        /// @brief Builds (or rebuilds) the combined GPU buffers (VBO, IBO, and IndirectBuffer).
        void BuildBatches();

        /// @brief Checks if the batch is dirty and rebuilds if necessary.
        void Update();

        /// @brief Issues the multi-draw call for the batch.
        void Render() const;

        /// @brief Sets the draw count of an object to zero (culls it).
        void CullObject(size_t objectIndex);

        /// @brief Updates the LOD for the specified object.
        void UpdateLOD(size_t objectIndex, size_t newLOD);

        // Accessors.
        [[nodiscard]] const std::string& GetShaderName() const { return shaderName_; }
        [[nodiscard]] int GetMaterialID() const { return materialID_; }
        [[nodiscard]] const MeshLayout& GetMeshLayout() const { return meshLayout_; }

    private:
        // Helper types.
        struct BatchGeometryTotals {
            int totalVertices = 0;
            int totalIndices = 0;
            int vertexElementCount = 0; // floats per vertex
        };

        // Helper functions.
        BatchGeometryTotals BuildLayoutAndTotals(VertexBufferLayout& vertexLayout) const;
        void CombineGeometryData(std::vector<float>& combinedVertexData,
            std::vector<GLuint>& combinedIndices,
            std::vector<std::vector<LODInfo>>& combinedLODInfos,
            std::vector<DrawElementsIndirectCommand>& combinedDrawCommands,
            GLuint& baseVertex) const;
        void CreateGpuBuffers(const VertexBufferLayout& vertexLayout,
            const std::vector<float>& vertexData,
            const std::vector<GLuint>& indexData,
            const std::vector<DrawElementsIndirectCommand>& drawCommands);

    private:
        std::string shaderName_;
        int materialID_;
        MeshLayout meshLayout_;

        // List of RenderObjects in the batch.
        std::vector<std::shared_ptr<BaseRenderObject>> renderObjects_;

        // GPU buffers for combined geometry.
        std::unique_ptr<VertexArray> vao_;
        std::unique_ptr<VertexBuffer> vertexBuffer_;   // Now stored as a member.
        std::unique_ptr<IndexBuffer> indexBuffer_;       // Now stored as a member.
        std::unique_ptr<IndirectBuffer> drawCommandBuffer_;

        // One draw command per object.
        std::vector<DrawElementsIndirectCommand> drawCommands_;
        // For each object, an array of LODInfo.
        std::vector<std::vector<LODInfo>> lodInfos_;

        // Flag indicating whether the batch data needs rebuilding.
        bool isDirty_ = true;
    };

} // namespace Graphics
