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
 * @brief A struct for one multi-draw command (used with glMultiDrawElementsIndirect).
 */
struct DrawElementsIndirectCommand
{
    GLuint count;         ///< Number of indices to draw
    GLuint instanceCount; ///< Number of instances
    GLuint firstIndex;    ///< Starting index in the IBO
    GLint  baseVertex;    ///< Adds this offset to indices in the VBO
    GLuint baseInstance;  ///< We can do per-instance data if needed
};

/**
 * @brief Info about a single LOD range. Tracks the index offset and index count
 *        within the combined IBO for that specific LOD.
 */
struct LODInfo
{
    size_t indexOffsetInCombinedBuffer = 0;
    size_t indexCount = 0;
};

/**
 * @brief A Batch groups multiple RenderObjects that share the same shader & material.
 *
 * It merges their vertex/index data into a single VBO/IBO and uses
 * glMultiDrawElementsIndirect to draw them in one (or few) calls.
 */
class Batch
{
public:
    Batch(const std::string& shaderName, int materialID);
    ~Batch();

    /**
     * @brief Add a RenderObject to this batch.
     *        This marks the batch as dirty (needing rebuild).
     */
    void AddRenderObject(const std::shared_ptr<BaseRenderObject>& renderObject);

    /**
     * @return The list of all render objects in this batch.
     */
    const std::vector<std::shared_ptr<BaseRenderObject>>& GetRenderObjects() const;

    /**
     * @brief Build or rebuild the combined GPU buffers (VBO, IBO, IndirectBuffer)
     *        from the attached render objects.
     */
    void BuildBatches();

    /**
     * @brief Check if dirty. If so, rebuild.
     */
    void Update();

    /**
     * @brief Issue the multi-draw call for all sub-objects in this batch.
     */
    void Render() const;

    // Culling
    void CullObject(size_t objectIndex);

    // LOD updates
    void UpdateLOD(size_t objectIndex, size_t newLOD);

    // Accessors
    [[nodiscard]] const std::string& GetShaderName() const { return m_ShaderName; }
    [[nodiscard]] int  GetMaterialID() const { return m_MaterialID; }
    [[nodiscard]] const MeshLayout& GetMeshLayout() const { return m_MeshLayout; }

private:
    std::string m_ShaderName;
    int         m_MaterialID;
    MeshLayout  m_MeshLayout;

    // The set of objects being batched
    std::vector<std::shared_ptr<BaseRenderObject>> m_RenderObjects;

    // GPU buffers for combined geometry
    std::unique_ptr<VertexArray>   m_VAO;
    std::unique_ptr<IndirectBuffer> m_DrawCommandBuffer;

    // One draw command per object
    std::vector<DrawElementsIndirectCommand> m_DrawCommands;
    // For each object, we store a vector of LOD ranges:
    //   m_LODInfos[objectIndex] => array of LODInfo for that object's mesh LODs
    std::vector<std::vector<LODInfo>> m_LODInfos;

    // Whether the batch data needs rebuilding
    bool m_IsDirty = true;
};