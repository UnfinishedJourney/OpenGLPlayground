#pragma once

#include <vector>
#include <memory>
#include <string>
#include <glad/glad.h>
#include "Renderer/RenderObject.h"
#include "Graphics/Buffers/VertexArray.h"
#include "Graphics/Buffers/VertexBuffer.h"
#include "Graphics/Buffers/IndexBuffer.h"
#include "Graphics/Buffers/VertexBufferLayout.h"
#include "Graphics/Meshes/MeshLayout.h"
#include "Graphics/Buffers/IndirectBuffer.h"

struct DrawElementsIndirectCommand {
    GLuint count;          // Number of indices
    GLuint instanceCount;  // Number of instances (1 for now)
    GLuint firstIndex;     // Offset into the index buffer
    GLint baseVertex;      // Offset into the vertex buffer
    GLuint baseInstance;   // First instance ID (0 for now)
};

struct LODInfo {
    size_t indexOffsetInCombinedBuffer; // Offset into the combined index buffer
    size_t indexCount;                  // Number of indices for this LOD
};

class Batch {
public:
    Batch(const std::string& shaderName, const std::string& materialName, const MeshLayout& meshLayout);
    ~Batch();

    // RenderObject Management
    void AddRenderObject(const std::shared_ptr<RenderObject>& renderObject);
    const std::vector<std::shared_ptr<RenderObject>>& GetRenderObjects() const;

    // Batch Operations
    void BuildBatches();
    void Update();
    void Render() const;

    // Culling and LOD
    void CullObject(size_t objectIndex);
    void UpdateLOD(size_t objectIndex, size_t newLOD);

    // Getters
    const std::string& GetShaderName() const;
    const std::string& GetMaterialName() const;
    const MeshLayout& GetMeshLayout() const;

private:
    void BuildBatch();

    std::string m_ShaderName;
    std::string m_MaterialName;
    MeshLayout m_MeshLayout;
    std::vector<std::shared_ptr<RenderObject>> m_RenderObjects;

    std::unique_ptr<VertexArray> m_VAO;
    std::shared_ptr<VertexBuffer> m_VBO;
    std::shared_ptr<IndexBuffer> m_IBO;

    std::unique_ptr<IndirectBuffer> m_DrawCommandBuffer;
    std::vector<DrawElementsIndirectCommand> m_DrawCommands;
    std::vector<std::vector<LODInfo>> m_LODInfos;

    bool m_IsDirty = true;
};