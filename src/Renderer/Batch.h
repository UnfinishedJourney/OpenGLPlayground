#pragma once

#include <memory>
#include <vector>
#include <string>
#include <glad/glad.h>
#include "Renderer/RenderObject.h"
#include "Graphics/Buffers/VertexArray.h"
#include "Graphics/Buffers/VertexBuffer.h"
#include "Graphics/Buffers/IndexBuffer.h"
#include "Graphics/Buffers/VertexBufferLayout.h"
#include "Graphics/Meshes/MeshLayout.h"
#include "Scene/Transform.h"
#include "Graphics/Buffers/IndirectBuffer.h"

/**
 * @brief A struct for one multi-draw command (used with glMultiDrawElementsIndirect).
 */
struct DrawElementsIndirectCommand
{
    GLuint count;
    GLuint instanceCount;
    GLuint firstIndex;
    GLint  baseVertex;
    GLuint baseInstance;
};

/**
 * @brief Info about a single LOD range: we track where in the global index buffer this LOD starts,
 * and how many indices it has.
 */
struct LODInfo
{
    size_t indexOffsetInCombinedBuffer = 0;
    size_t indexCount = 0;
};

/**
 * @brief A Batch groups multiple RenderObjects that share the same shader, material, and mesh layout.
 * It then merges their data into combined VBO/IBO, and issues a multi-draw command.
 */
class Batch
{
public:
    Batch(const std::string& shaderName,
        int materialID,
        const Transform transform);
    ~Batch();

    void AddRenderObject(const std::shared_ptr<RenderObject>& renderObject);
    const std::vector<std::shared_ptr<RenderObject>>& GetRenderObjects() const;

    /**
     * @brief Build the combined VBO/IBO for all RenderObjects.
     *        Also populates the multi-draw commands with the correct offsets.
     */
    void BuildBatches();

    /**
     * @brief If the batch is dirty, rebuild it; otherwise, do nothing.
     */
    void Update();

    /**
     * @brief Issue a multi-draw call for all commands in this batch.
     */
    void Render() const;

    // Culling & LOD
    void CullObject(size_t objectIndex);
    void UpdateLOD(size_t objectIndex, size_t newLOD);

    // Accessors
    const std::string& GetShaderName() const;
    int GetMaterialID() const;
    const MeshLayout& GetMeshLayout() const;
    const Transform& GetTransform() const;

private:

    const std::string                           m_ShaderName;
    int                                         m_MaterialID;
    MeshLayout                                  m_MeshLayout;
    const Transform                             m_Transform;


    std::vector<std::shared_ptr<RenderObject>> m_RenderObjects;

    std::unique_ptr<VertexArray>   m_VAO;
    std::unique_ptr<IndirectBuffer> m_DrawCommandBuffer;

    std::vector<DrawElementsIndirectCommand> m_DrawCommands;
    std::vector<std::vector<LODInfo>>        m_LODInfos;

    bool m_IsDirty = true;
};