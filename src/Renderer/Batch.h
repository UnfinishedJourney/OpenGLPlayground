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
        GLuint count;         ///< Number of indices to draw.
        GLuint instanceCount; ///< Number of instances.
        GLuint firstIndex;    ///< Starting index in the IBO.
        GLint  baseVertex;    ///< Offset added to indices.
        GLuint baseInstance;  ///< Base instance ID.
    };

    /**
     * @brief Info about one LOD range. Tracks the index offset and count within the combined IBO.
     */
    struct LODInfo {
        size_t indexOffsetInCombinedBuffer = 0;
        size_t indexCount = 0;
    };

    /**
     * @brief Batch groups multiple RenderObjects sharing the same shader and material.
     *
     * It merges their vertex/index data into single GPU buffers (VBO, IBO, and indirect draw buffer)
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

        /// @brief Builds (or rebuilds) the combined GPU buffers (VBO, IBO, IndirectBuffer).
        void BuildBatches();

        /// @brief Checks if the batch is dirty, and rebuilds if necessary.
        void Update();

        /// @brief Issues the multi-draw call for the batch.
        void Render() const;

        // Culling
        void CullObject(size_t objectIndex);

        // LOD updates
        void UpdateLOD(size_t objectIndex, size_t newLOD);

        // Accessors.
        [[nodiscard]] const std::string& GetShaderName() const { return shaderName_; }
        [[nodiscard]] int GetMaterialID() const { return materialID_; }
        [[nodiscard]] const MeshLayout& GetMeshLayout() const { return meshLayout_; }

    private:
        std::string shaderName_;
        int materialID_;
        MeshLayout meshLayout_;

        // List of RenderObjects in the batch.
        std::vector<std::shared_ptr<BaseRenderObject>> renderObjects_;

        // GPU buffers for combined geometry.
        std::unique_ptr<VertexArray> VAO_;
        std::unique_ptr<IndirectBuffer> drawCommandBuffer_;

        // One draw command per object.
        std::vector<DrawElementsIndirectCommand> drawCommands_;
        // For each object, an array of LODInfo.
        std::vector<std::vector<LODInfo>> LODInfos_;

        // Flag indicating whether the batch data needs rebuilding.
        bool isDirty_ = true;
    };

} // namespace Graphics