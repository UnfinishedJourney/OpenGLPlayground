#include "Batch.h"
#include "Utilities/Logger.h"
#include <numeric>
#include <span>

Batch::Batch(const std::string& shaderName,
    const std::string& materialName,
    const MeshLayout& meshLayout)
    : m_ShaderName(shaderName),
    m_MaterialName(materialName),
    m_MeshLayout(meshLayout),
    m_VAO(std::make_unique<VertexArray>())
{
}

Batch::~Batch() {
    // RAII cleanup
}

void Batch::AddRenderObject(const std::shared_ptr<RenderObject>& renderObject) {
    m_RenderObjects.push_back(renderObject);
    m_IsDirty = true;
}

const std::vector<std::shared_ptr<RenderObject>>& Batch::GetRenderObjects() const {
    return m_RenderObjects;
}

const std::string& Batch::GetShaderName() const {
    return m_ShaderName;
}

const std::string& Batch::GetMaterialName() const {
    return m_MaterialName;
}

const MeshLayout& Batch::GetMeshLayout() const {
    return m_MeshLayout;
}

void Batch::BuildBatches() {
    if (m_RenderObjects.empty()) {
        Logger::GetLogger()->warn("No RenderObjects in batch for {} / {}",
            m_ShaderName, m_MaterialName);
        return;
    }

    // Vertex layout
    VertexBufferLayout vertexBufferLayout;
    GLuint attribIndex = 0;

    if (m_MeshLayout.hasPositions)   vertexBufferLayout.Push<float>(3, attribIndex++);
    if (m_MeshLayout.hasNormals)     vertexBufferLayout.Push<float>(3, attribIndex++);
    if (m_MeshLayout.hasTangents)    vertexBufferLayout.Push<float>(3, attribIndex++);
    if (m_MeshLayout.hasBitangents)  vertexBufferLayout.Push<float>(3, attribIndex++);
    for (auto& texType : m_MeshLayout.textureTypes) {
        vertexBufferLayout.Push<float>(2, attribIndex++);
    }

    std::vector<float> combinedVertexData;
    std::vector<GLuint> combinedIndices;
    m_LODInfos.clear();
    m_DrawCommands.clear();

    GLuint baseVertex = 0;

    for (auto& ro : m_RenderObjects) {
        const auto& mesh = ro->GetMesh();
        std::vector<LODInfo> lodInfos;

        size_t vertexCount = mesh->positions.size();

        // Fill vertex data
        for (size_t i = 0; i < vertexCount; i++) {
            if (m_MeshLayout.hasPositions && i < mesh->positions.size()) {
                const auto& pos = mesh->positions[i];
                combinedVertexData.insert(combinedVertexData.end(),
                    { pos.x, pos.y, pos.z });
            }
            if (m_MeshLayout.hasNormals && i < mesh->normals.size()) {
                const auto& n = mesh->normals[i];
                combinedVertexData.insert(combinedVertexData.end(),
                    { n.x, n.y, n.z });
            }
            if (m_MeshLayout.hasTangents && i < mesh->tangents.size()) {
                const auto& t = mesh->tangents[i];
                combinedVertexData.insert(combinedVertexData.end(),
                    { t.x, t.y, t.z });
            }
            if (m_MeshLayout.hasBitangents && i < mesh->bitangents.size()) {
                const auto& b = mesh->bitangents[i];
                combinedVertexData.insert(combinedVertexData.end(),
                    { b.x, b.y, b.z });
            }
            for (auto& texType : m_MeshLayout.textureTypes) {
                auto uvIt = mesh->uvs.find(texType);
                if (uvIt != mesh->uvs.end() && i < uvIt->second.size()) {
                    const auto& uv = uvIt->second[i];
                    combinedVertexData.insert(combinedVertexData.end(),
                        { uv.x, uv.y });
                }
                else {
                    combinedVertexData.insert(combinedVertexData.end(), { 0.f, 0.f });
                }
            }
        }

        // LOD building
        for (auto& lod : mesh->lods) {
            LODInfo lodInfo;
            lodInfo.indexOffsetInCombinedBuffer = combinedIndices.size();
            lodInfo.indexCount = lod.indexCount;

            // Copy indices
            for (size_t idx = 0; idx < lod.indexCount; idx++) {
                GLuint index = mesh->indices[lod.indexOffset + idx];
                combinedIndices.push_back(index + baseVertex);
            }
            lodInfos.push_back(lodInfo);
        }

        m_LODInfos.push_back(lodInfos);

        // Create draw command based on current LOD
        size_t initialLOD = ro->GetCurrentLOD();
        if (initialLOD >= lodInfos.size()) {
            initialLOD = 0;
        }
        const LODInfo& lodInfo = lodInfos[initialLOD];

        DrawElementsIndirectCommand cmd = {};
        cmd.count = static_cast<GLuint>(lodInfo.indexCount);
        cmd.instanceCount = 1;
        cmd.firstIndex = static_cast<GLuint>(lodInfo.indexOffsetInCombinedBuffer);
        cmd.baseVertex = 0;
        cmd.baseInstance = 0;

        m_DrawCommands.push_back(cmd);

        baseVertex += static_cast<GLuint>(vertexCount);
    }

    // Create/Update VBO
    {
        std::span<const std::byte> vertSpan(
            reinterpret_cast<const std::byte*>(combinedVertexData.data()),
            combinedVertexData.size() * sizeof(float)
        );
        if (!m_VBO) {
            m_VBO = std::make_shared<VertexBuffer>(vertSpan, GL_STATIC_DRAW);
        }
        else {
            m_VBO->UpdateData(vertSpan);
        }
    }

    // Create/Update IBO
    {
        std::span<const GLuint> idxSpan(combinedIndices.data(), combinedIndices.size());
        if (!m_IBO) {
            m_IBO = std::make_shared<IndexBuffer>(idxSpan, GL_STATIC_DRAW);
        }
        else {
            m_IBO->UpdateData(idxSpan);
        }
    }

    // Create/Update Indirect Buffer
    {
        std::span<const std::byte> cmdSpan(
            reinterpret_cast<const std::byte*>(m_DrawCommands.data()),
            m_DrawCommands.size() * sizeof(DrawElementsIndirectCommand)
        );
        if (!m_DrawCommandBuffer) {
            m_DrawCommandBuffer = std::make_unique<IndirectBuffer>(cmdSpan, GL_DYNAMIC_DRAW);
        }
        else {
            m_DrawCommandBuffer->UpdateData(cmdSpan);
        }
    }

    // Setup VAO
    {
        m_VAO->Bind();
        m_VAO->AddBuffer(*m_VBO, vertexBufferLayout);
        m_VAO->SetIndexBuffer(*m_IBO);
        m_VAO->Unbind();
    }

    m_IsDirty = false;
}

void Batch::BuildBatch() {
    BuildBatches();
}

void Batch::Update() {
    if (m_IsDirty) {
        BuildBatches();
    }
}

void Batch::Render() const {
    if (m_DrawCommands.empty()) return;

    m_VAO->Bind();
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_DrawCommandBuffer->GetRendererID());
    glMultiDrawElementsIndirect(
        GL_TRIANGLES,
        GL_UNSIGNED_INT,
        nullptr,
        static_cast<GLsizei>(m_DrawCommands.size()),
        0
    );
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
    m_VAO->Unbind();
}

void Batch::CullObject(size_t objectIndex) {
    if (objectIndex >= m_DrawCommands.size()) {
        Logger::GetLogger()->error("CullObject: invalid index");
        return;
    }
    auto& cmd = m_DrawCommands[objectIndex];
    if (cmd.count == 0) {
        // Already culled
        return;
    }
    cmd.count = 0;

    // Update in the GPU buffer
    std::span<const std::byte> cmdSpan(
        reinterpret_cast<const std::byte*>(&cmd),
        sizeof(DrawElementsIndirectCommand)
    );
    GLintptr offset = objectIndex * sizeof(DrawElementsIndirectCommand);
    m_DrawCommandBuffer->UpdateData(cmdSpan, offset);
}

void Batch::UpdateLOD(size_t objectIndex, size_t newLOD) {
    if (objectIndex >= m_RenderObjects.size() || objectIndex >= m_LODInfos.size()) {
        Logger::GetLogger()->error("UpdateLOD: invalid index");
        return;
    }

    auto ro = m_RenderObjects[objectIndex];
    const auto& lodInfos = m_LODInfos[objectIndex];
    if (newLOD >= lodInfos.size()) {
        Logger::GetLogger()->warn("LOD index out of range, skipping");
        return;
    }

    // If the RenderObject is already at newLOD, do nothing
    if (ro->GetCurrentLOD() == newLOD) {
        return;
    }
    ro->SetLOD(newLOD);

    auto& cmd = m_DrawCommands[objectIndex];
    const LODInfo& lodInfo = lodInfos[newLOD];

    // If the command is already correct, skip
    if (cmd.count == lodInfo.indexCount && cmd.firstIndex == lodInfo.indexOffsetInCombinedBuffer) {
        return;
    }

    // Update the draw command
    cmd.count = static_cast<GLuint>(lodInfo.indexCount);
    cmd.firstIndex = static_cast<GLuint>(lodInfo.indexOffsetInCombinedBuffer);

    // Update GPU
    std::span<const std::byte> cmdSpan(
        reinterpret_cast<const std::byte*>(&cmd),
        sizeof(DrawElementsIndirectCommand)
    );
    GLintptr offset = objectIndex * sizeof(DrawElementsIndirectCommand);
    m_DrawCommandBuffer->UpdateData(cmdSpan, offset);
}