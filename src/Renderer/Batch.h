#pragma once

#include <memory>
#include <vector>
#include <string>
#include <glad/glad.h>
#include <cstddef>

#include "Renderer/RenderObject.h"
#include "Graphics/Buffers/VertexArray.h"
#include "Graphics/Buffers/VertexBuffer.h"
#include "Graphics/Buffers/IndexBuffer.h"
#include "Graphics/Buffers/VertexBufferLayout.h"
#include "Graphics/Buffers/IndirectBuffer.h"
#include "Graphics/Meshes/MeshLayout.h"

/**
 * @brief Structure for one multi-draw command (used with glMultiDrawElementsIndirect).
 */
struct DrawElementsIndirectCommand {
    GLuint count;         ///< Number of indices to draw
    GLuint instanceCount; ///< Number of instances
    GLuint firstIndex;    ///< Starting index in the IBO
    GLint  baseVertex;    ///< Offset added to indices in the VBO
    GLuint baseInstance;  ///< Base instance ID
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
 * It merges their vertex/index data into single GPU buffers (VBO, IBO)
 * and uses glMultiDrawElementsIndirect to issue a multi-draw call.
 */
class Batch {
public:
    Batch(const std::string& shaderName, int materialID);
    ~Batch();

    /**
     * @brief Add a RenderObject to the batch.
     */
    void AddRenderObject(const std::shared_ptr<BaseRenderObject>& renderObject);

    /**
     * @brief Returns the list of RenderObjects in the batch.
     */
    const std::vector<std::shared_ptr<BaseRenderObject>>& GetRenderObjects() const;

    /**
     * @brief Build (or rebuild) the combined GPU buffers (VBO, IBO, IndirectBuffer).
     */
    void BuildBatches();

    /**
     * @brief Check if the batch is dirty, and rebuild if necessary.
     */
    void Update();

    /**
     * @brief Issue the multi-draw call for the batch.
     */
    void Render() const;

    // Culling
    void CullObject(size_t objectIndex);

    // LOD updates
    void UpdateLOD(size_t objectIndex, size_t newLOD);

    // Accessors
    [[nodiscard]] const std::string& GetShaderName() const { return m_ShaderName; }
    [[nodiscard]] int GetMaterialID() const { return m_MaterialID; }
    [[nodiscard]] const MeshLayout& GetMeshLayout() const { return m_MeshLayout; }

private:
    std::string m_ShaderName;
    int m_MaterialID;
    MeshLayout m_MeshLayout;

    // List of objects in the batch.
    std::vector<std::shared_ptr<BaseRenderObject>> m_RenderObjects;

    // GPU buffers for combined geometry.
    std::unique_ptr<Graphics::VertexArray> m_VAO;
    std::unique_ptr<Graphics::IndirectBuffer> m_DrawCommandBuffer;

    // One draw command per object.
    std::vector<DrawElementsIndirectCommand> m_DrawCommands;
    // For each object, an array of LODInfo.
    std::vector<std::vector<LODInfo>> m_LODInfos;

    // Flag indicating whether the batch data needs rebuilding.
    bool m_IsDirty = true;
};