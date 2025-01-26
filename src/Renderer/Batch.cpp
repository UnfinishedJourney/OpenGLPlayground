#include "Batch.h"
#include "Utilities/Logger.h"
#include <numeric>
#include <span>
#include "Resources/ResourceManager.h"

Batch::Batch(const std::string& shaderName,
    int materialID)
    : m_ShaderName(shaderName)
    , m_MaterialID(materialID)
    , m_VAO(std::make_unique<VertexArray>())
    , m_IsDirty(true)
{
    auto [mel, mal] = ResourceManager::GetInstance().getLayoutsFromShader(shaderName);
    m_MeshLayout = mel;
}

Batch::~Batch()
{
}

void Batch::AddRenderObject(const std::shared_ptr<BaseRenderObject>& renderObject)
{
    m_RenderObjects.push_back(renderObject);
    m_IsDirty = true;
}

const std::vector<std::shared_ptr<BaseRenderObject>>& Batch::GetRenderObjects() const
{
    return m_RenderObjects;
}

const std::string& Batch::GetShaderName() const
{
    return m_ShaderName;
}

int Batch::GetMaterialID() const
{
    return m_MaterialID;
}

const MeshLayout& Batch::GetMeshLayout() const
{
    return m_MeshLayout;
}

void Batch::BuildBatches()
{
    if (m_RenderObjects.empty()) {
        Logger::GetLogger()->warn("No RenderObjects to batch for shader '{}' material '{}'.",
            m_ShaderName, m_MaterialID);
        return;
    }

    // 1) Create a VertexBufferLayout based on the MeshLayout
    VertexBufferLayout vertexBufferLayout;
    GLuint attribIndex = 0;

    if (m_MeshLayout.hasPositions)   vertexBufferLayout.Push<float>(3, attribIndex++);
    if (m_MeshLayout.hasNormals)     vertexBufferLayout.Push<float>(3, attribIndex++);
    if (m_MeshLayout.hasTangents)    vertexBufferLayout.Push<float>(3, attribIndex++);
    if (m_MeshLayout.hasBitangents)  vertexBufferLayout.Push<float>(3, attribIndex++);

    for (auto& texType : m_MeshLayout.textureTypes) {
        (void)texType; // we just know each texture set => 2 floats
        vertexBufferLayout.Push<float>(2, attribIndex++);
    }

    // 2) Combine all vertex data and index data
    std::vector<float>  combinedVertexData;
    std::vector<GLuint> combinedIndices;
    m_LODInfos.clear();
    m_DrawCommands.clear();

    // For multi-draw, each RenderObject will create 1 command
    combinedVertexData.reserve(m_RenderObjects.size() * 500); // heuristic
    combinedIndices.reserve(m_RenderObjects.size() * 1000);

    GLuint baseVertex = 0; // running count of total vertices so far

    for (auto& ro : m_RenderObjects)
    {
        auto mesh = ro->GetMesh();
        size_t vtxCount = mesh->positions.size();

        // Append vertex data
        for (size_t i = 0; i < vtxCount; i++)
        {
            if (m_MeshLayout.hasPositions && i < mesh->positions.size()) {
                auto& pos = mesh->positions[i];
                combinedVertexData.insert(combinedVertexData.end(), { pos.x, pos.y, pos.z });
            }

            if (m_MeshLayout.hasNormals && i < mesh->normals.size()) {
                auto& n = mesh->normals[i];
                combinedVertexData.insert(combinedVertexData.end(), { n.x, n.y, n.z });
            }

            if (m_MeshLayout.hasTangents && i < mesh->tangents.size()) {
                auto& t = mesh->tangents[i];
                combinedVertexData.insert(combinedVertexData.end(), { t.x, t.y, t.z });
            }

            if (m_MeshLayout.hasBitangents && i < mesh->bitangents.size()) {
                auto& b = mesh->bitangents[i];
                combinedVertexData.insert(combinedVertexData.end(), { b.x, b.y, b.z });
            }

            // for each requested texture channel, add 2 floats
            for (auto texType : m_MeshLayout.textureTypes) {
                auto uvIt = mesh->uvs.find(texType);
                if (uvIt != mesh->uvs.end() && i < uvIt->second.size()) {
                    auto& uv = uvIt->second[i];
                    combinedVertexData.insert(combinedVertexData.end(), { uv.x, uv.y });
                }
                else {
                    // no uv => fallback 0,0
                    combinedVertexData.insert(combinedVertexData.end(), { 0.f, 0.f });
                }
            }
        }

        // Build LODInfos
        std::vector<LODInfo> lodInfos;
        lodInfos.reserve(mesh->lods.size());
        for (auto& lod : mesh->lods)
        {
            LODInfo lodInfo;
            lodInfo.indexOffsetInCombinedBuffer = combinedIndices.size();
            lodInfo.indexCount = lod.indexCount;

            // copy indices
            for (size_t idx = 0; idx < lod.indexCount; ++idx) {
                GLuint index = mesh->indices[lod.indexOffset + idx];
                combinedIndices.push_back(index + baseVertex);
            }
            lodInfos.push_back(lodInfo);
        }
        m_LODInfos.push_back(lodInfos);

        // the command uses the current LOD to decide which indices
        size_t initialLOD = ro->GetCurrentLOD();
        if (initialLOD >= lodInfos.size()) {
            initialLOD = 0;
        }
        auto& lodRef = lodInfos[initialLOD];

        DrawElementsIndirectCommand cmd{};
        cmd.count = static_cast<GLuint>(lodRef.indexCount);
        cmd.instanceCount = 1;
        cmd.firstIndex = static_cast<GLuint>(lodRef.indexOffsetInCombinedBuffer);
        cmd.baseVertex = 0; // We added baseVertex to the indices themselves
        cmd.baseInstance = 0;

        m_DrawCommands.push_back(cmd);

        baseVertex += static_cast<GLuint>(vtxCount);
    }

    // 3) Create/Update GPU buffers
    std::span<const std::byte> vertSpan(
        reinterpret_cast<const std::byte*>(combinedVertexData.data()),
        combinedVertexData.size() * sizeof(float)
    );

    auto m_VBO = std::make_unique<VertexBuffer>(vertSpan, GL_STATIC_DRAW);
    std::span<const GLuint> idxSpan(combinedIndices.data(), combinedIndices.size());
    auto m_IBO = std::make_unique<IndexBuffer>(idxSpan, GL_STATIC_DRAW);

    // Indirect command buffer
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

    // 4) Set up the VAO
    m_VAO->Bind();
    m_VAO->AddBuffer(*m_VBO, vertexBufferLayout);
    m_VAO->SetIndexBuffer(*m_IBO);
    m_VAO->Unbind();

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

void Batch::CullObject(size_t objectIndex)
{
    if (objectIndex >= m_DrawCommands.size()) {
        Logger::GetLogger()->error("CullObject: invalid index {}", objectIndex);
        return;
    }
    auto& cmd = m_DrawCommands[objectIndex];
    if (cmd.count == 0) {
        // Already culled
        return;
    }

    cmd.count = 0; // zeroing out means "draw nothing"

    // Update GPU
    std::span<const std::byte> cmdSpan(
        reinterpret_cast<const std::byte*>(&cmd),
        sizeof(DrawElementsIndirectCommand)
    );
    GLintptr offset = objectIndex * sizeof(DrawElementsIndirectCommand);
    m_DrawCommandBuffer->UpdateData(cmdSpan, offset);
}

void Batch::UpdateLOD(size_t objectIndex, size_t newLOD)
{
    if (objectIndex >= m_RenderObjects.size() ||
        objectIndex >= m_LODInfos.size())
    {
        Logger::GetLogger()->error("UpdateLOD: invalid index {}.", objectIndex);
        return;
    }
    auto ro = m_RenderObjects[objectIndex];
    const auto& lods = m_LODInfos[objectIndex];

    if (newLOD >= lods.size()) {
        Logger::GetLogger()->warn("LOD {} out of range for object index {}. Skipped.", newLOD, objectIndex);
        return;
    }
    if (ro->GetCurrentLOD() == newLOD) {
        return; // no change
    }
    ro->SetLOD(newLOD);

    auto& cmd = m_DrawCommands[objectIndex];
    const LODInfo& lodRef = lods[newLOD];

    cmd.count = static_cast<GLuint>(lodRef.indexCount);
    cmd.firstIndex = static_cast<GLuint>(lodRef.indexOffsetInCombinedBuffer);

    // Update GPU
    std::span<const std::byte> cmdSpan(
        reinterpret_cast<const std::byte*>(&cmd),
        sizeof(DrawElementsIndirectCommand)
    );
    GLintptr offset = objectIndex * sizeof(DrawElementsIndirectCommand);
    m_DrawCommandBuffer->UpdateData(cmdSpan, offset);
}