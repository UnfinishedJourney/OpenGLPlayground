#include "Batch.h"
#include "Utilities/Logger.h"
#include "Resources/ResourceManager.h"

#include <span>
#include <numeric>
#include <stdexcept>

// Constructor: Retrieves the mesh layout from ResourceManager.
Batch::Batch(const std::string& shaderName, int materialID)
    : m_ShaderName(shaderName)
    , m_MaterialID(materialID)
    , m_VAO(std::make_unique<VertexArray>())
    , m_IsDirty(true)
{
    auto [meshLayout, materialLayout] = ResourceManager::GetInstance().getLayoutsFromShader(shaderName);
    m_MeshLayout = meshLayout;
}

Batch::~Batch() = default;

void Batch::AddRenderObject(const std::shared_ptr<BaseRenderObject>& renderObject)
{
    if (!renderObject) {
        Logger::GetLogger()->error("Batch::AddRenderObject: received a null render object.");
        return;
    }
    m_RenderObjects.push_back(renderObject);
    m_IsDirty = true;
}

const std::vector<std::shared_ptr<BaseRenderObject>>& Batch::GetRenderObjects() const
{
    return m_RenderObjects;
}

void Batch::BuildBatches()
{
    if (m_RenderObjects.empty()) {
        Logger::GetLogger()->warn("Batch::BuildBatches: no RenderObjects in batch (shader='{}', matID={}).",
            m_ShaderName, m_MaterialID);
        return;
    }

    // 1) Build the VertexBufferLayout from the mesh layout.
    VertexBufferLayout vertexLayout;
    GLuint attribIndex = 0;
    if (m_MeshLayout.hasPositions)
        vertexLayout.Push<float>(3, attribIndex++);
    if (m_MeshLayout.hasNormals)
        vertexLayout.Push<float>(3, attribIndex++);
    if (m_MeshLayout.hasTangents)
        vertexLayout.Push<float>(3, attribIndex++);
    if (m_MeshLayout.hasBitangents)
        vertexLayout.Push<float>(3, attribIndex++);
    for (auto texType : m_MeshLayout.textureTypes) {
        vertexLayout.Push<float>(2, attribIndex++);
    }

    // 2) Combine all vertex and index data.
    std::vector<float> combinedVertexData;
    std::vector<GLuint> combinedIndices;
    m_LODInfos.clear();
    m_DrawCommands.clear();

    m_LODInfos.reserve(m_RenderObjects.size());
    m_DrawCommands.reserve(m_RenderObjects.size());
    combinedVertexData.reserve(m_RenderObjects.size() * 500);
    combinedIndices.reserve(m_RenderObjects.size() * 1000);

    GLuint baseVertex = 0; // Running offset.
    for (auto& ro : m_RenderObjects) {
        auto mesh = ro->GetMesh();
        if (!mesh) {
            Logger::GetLogger()->error("Batch::BuildBatches: RenderObject has no valid mesh.");
            continue;
        }
        const size_t vCount = mesh->positions.size();

        // 2a) Append vertex data (positions, normals, tangents, bitangents, and texture coords).
        for (size_t i = 0; i < vCount; i++) {
            if (m_MeshLayout.hasPositions && i < mesh->positions.size()) {
                const auto& pos = mesh->positions[i];
                combinedVertexData.insert(combinedVertexData.end(), { pos.x, pos.y, pos.z });
            }
            if (m_MeshLayout.hasNormals && i < mesh->normals.size()) {
                const auto& nor = mesh->normals[i];
                combinedVertexData.insert(combinedVertexData.end(), { nor.x, nor.y, nor.z });
            }
            if (m_MeshLayout.hasTangents && i < mesh->tangents.size()) {
                const auto& tan = mesh->tangents[i];
                combinedVertexData.insert(combinedVertexData.end(), { tan.x, tan.y, tan.z });
            }
            if (m_MeshLayout.hasBitangents && i < mesh->bitangents.size()) {
                const auto& bitan = mesh->bitangents[i];
                combinedVertexData.insert(combinedVertexData.end(), { bitan.x, bitan.y, bitan.z });
            }
            for (auto texType : m_MeshLayout.textureTypes) {
                const auto uvIt = mesh->uvs.find(texType);
                if (uvIt != mesh->uvs.end() && i < uvIt->second.size()) {
                    const auto& uv = uvIt->second[i];
                    combinedVertexData.insert(combinedVertexData.end(), { uv.x, uv.y });
                }
                else {
                    combinedVertexData.insert(combinedVertexData.end(), { 0.f, 0.f });
                }
            }
        }

        // 2b) Build LODInfo array for this object.
        std::vector<LODInfo> lodInfos;
        lodInfos.reserve(mesh->lods.size());
        for (auto& lod : mesh->lods) {
            LODInfo info;
            info.indexOffsetInCombinedBuffer = combinedIndices.size();
            info.indexCount = lod.indexCount;
            for (size_t idx = 0; idx < lod.indexCount; ++idx) {
                GLuint oldIndex = mesh->indices[lod.indexOffset + idx];
                combinedIndices.push_back(oldIndex + baseVertex);
            }
            lodInfos.push_back(info);
        }
        m_LODInfos.push_back(std::move(lodInfos));

        // 2c) Create an indirect draw command for this object.
        size_t lodUsed = ro->GetCurrentLOD();
        if (lodUsed >= m_LODInfos.back().size())
            lodUsed = 0;
        auto& usedLOD = m_LODInfos.back()[lodUsed];

        DrawElementsIndirectCommand cmd{};
        cmd.count = static_cast<GLuint>(usedLOD.indexCount);
        cmd.instanceCount = 1;
        cmd.firstIndex = static_cast<GLuint>(usedLOD.indexOffsetInCombinedBuffer);
        cmd.baseVertex = 0;
        cmd.baseInstance = 0;
        m_DrawCommands.push_back(cmd);

        baseVertex += static_cast<GLuint>(vCount);
    }

    // 3) Create and/or update the GPU buffers.
    {
        // Create vertex buffer.
        std::span<const std::byte> vertexSpan(
            reinterpret_cast<const std::byte*>(combinedVertexData.data()),
            combinedVertexData.size() * sizeof(float)
        );
        auto vb = std::make_unique<VertexBuffer>(vertexSpan, GL_STATIC_DRAW);

        // Create index buffer.
        std::span<const GLuint> indexSpan(
            combinedIndices.data(),
            combinedIndices.size()
        );
        auto ib = std::make_unique<IndexBuffer>(indexSpan, GL_STATIC_DRAW);

        // Create or update the indirect command buffer.
        std::span<const std::byte> cmdSpan(
            reinterpret_cast<const std::byte*>(m_DrawCommands.data()),
            m_DrawCommands.size() * sizeof(DrawElementsIndirectCommand)
        );
        if (!m_DrawCommandBuffer) {
            m_DrawCommandBuffer = std::make_unique<IndirectBuffer>();
            m_DrawCommandBuffer->SetData(cmdSpan, GL_DYNAMIC_DRAW);
            if (m_DrawCommandBuffer->GetRendererID() == 0) {
                Logger::GetLogger()->error("Batch::BuildBatches: failed to create valid indirect buffer.");
            }
        }
        else {
            if (m_DrawCommandBuffer->GetBufferSize() != cmdSpan.size_bytes()) {
                m_DrawCommandBuffer->SetData(cmdSpan, GL_DYNAMIC_DRAW);
            }
            else {
                m_DrawCommandBuffer->UpdateData(cmdSpan);
            }
        }

        // Build the VAO.
        m_VAO->Bind();
        m_VAO->AddBuffer(*vb, vertexLayout);
        m_VAO->SetIndexBuffer(*ib);
        m_VAO->Unbind();
    }

    m_IsDirty = false;
}

void Batch::Update()
{
    if (m_IsDirty) {
        BuildBatches();
    }
}

void Batch::Render() const
{
    if (m_DrawCommands.empty() || !m_DrawCommandBuffer)
        return; // Nothing to draw

    m_VAO->Bind();
    // Bind the indirect draw buffer
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_DrawCommandBuffer->GetRendererID());
    // Note: set stride to sizeof(DrawElementsIndirectCommand) so the driver knows how to iterate the commands.
    glMultiDrawElementsIndirect(
        GL_TRIANGLES,
        GL_UNSIGNED_INT,
        nullptr,
        static_cast<GLsizei>(m_DrawCommands.size()),
        sizeof(DrawElementsIndirectCommand)
    );
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
    m_VAO->Unbind();
}

void Batch::CullObject(size_t objectIndex)
{
    if (objectIndex >= m_DrawCommands.size()) {
        Logger::GetLogger()->error("Batch::CullObject: objectIndex={} out of range.", objectIndex);
        return;
    }
    // Set count to 0 so that the object is not drawn.
    auto& cmd = m_DrawCommands[objectIndex];
    if (cmd.count != 0) {
        cmd.count = 0;
        std::span<const std::byte> cmdSpan(
            reinterpret_cast<const std::byte*>(&cmd),
            sizeof(DrawElementsIndirectCommand)
        );
        GLintptr offset = objectIndex * sizeof(DrawElementsIndirectCommand);
        m_DrawCommandBuffer->UpdateData(cmdSpan, offset);
    }
}

void Batch::UpdateLOD(size_t objectIndex, size_t newLOD)
{
    if (objectIndex >= m_RenderObjects.size() || objectIndex >= m_LODInfos.size()) {
        Logger::GetLogger()->error("Batch::UpdateLOD: invalid objectIndex={}.", objectIndex);
        return;
    }
    auto ro = m_RenderObjects[objectIndex];
    if (!ro->SetLOD(newLOD))
        return; // No change

    size_t lodUsed = ro->GetCurrentLOD();
    if (lodUsed >= m_LODInfos[objectIndex].size())
        lodUsed = 0;
    auto& lodRef = m_LODInfos[objectIndex][lodUsed];

    auto& cmd = m_DrawCommands[objectIndex];
    cmd.count = static_cast<GLuint>(lodRef.indexCount);
    cmd.firstIndex = static_cast<GLuint>(lodRef.indexOffsetInCombinedBuffer);

    std::span<const std::byte> cmdSpan(
        reinterpret_cast<const std::byte*>(&cmd),
        sizeof(DrawElementsIndirectCommand)
    );
    GLintptr offset = objectIndex * sizeof(DrawElementsIndirectCommand);
    m_DrawCommandBuffer->UpdateData(cmdSpan, offset);
}