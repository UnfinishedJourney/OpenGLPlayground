#include "Renderer/Batch.h"
#include "Utilities/Logger.h"
#include <variant>
#include <numeric>
#include <algorithm>

Batch::Batch(const std::string& shaderName, const std::string& materialName, const MeshLayout& meshLayout)
    : m_ShaderName(shaderName),
    m_MaterialName(materialName),
    m_MeshLayout(meshLayout),
    m_VAO(std::make_unique<VertexArray>()) {
}

Batch::~Batch() {
    // automatically cleaned
}

void Batch::AddRenderObject(const std::shared_ptr<RenderObject>& renderObject) {
    m_RenderObjects.push_back(renderObject);
    m_IsDirty = true;
}

void Batch::Update() {
    if (!m_IsDirty) return;
    BuildBatch();
    m_IsDirty = false;
}

void Batch::BuildBatch() {
    if (m_RenderObjects.empty()) {
        Logger::GetLogger()->warn("No RenderObjects to batch.");
        return;
    }

    // Build vertex layout
    VertexBufferLayout vertexBufferLayout;
    GLuint attributeIndex = 0;

    size_t positionComponentCount = 3;
    const auto& firstMesh = m_RenderObjects.front()->GetMesh();
    std::visit([&](const auto& positionsVec) {
        using VecType = typename std::decay_t<decltype(positionsVec)>::value_type;
        if constexpr (std::is_same_v<VecType, glm::vec2>) {
            positionComponentCount = 2;
        }
        }, firstMesh->positions);

    if (m_MeshLayout.hasPositions) {
        vertexBufferLayout.Push<float>((GLuint)positionComponentCount, attributeIndex++);
    }
    if (m_MeshLayout.hasNormals) {
        vertexBufferLayout.Push<float>(3, attributeIndex++);
    }
    if (m_MeshLayout.hasTangents) {
        vertexBufferLayout.Push<float>(3, attributeIndex++);
    }
    if (m_MeshLayout.hasBitangents) {
        vertexBufferLayout.Push<float>(3, attributeIndex++);
    }
    for (const auto& texType : m_MeshLayout.textureTypes) {
        vertexBufferLayout.Push<float>(2, attributeIndex++);
    }

    std::vector<float> combinedVertexData;
    std::vector<GLuint> combinedIndices;
    m_LODInfos.clear();
    m_DrawCommands.clear();

    GLuint baseVertex = 0;

    for (const auto& ro : m_RenderObjects) {
        const auto& mesh = ro->GetMesh();
        std::vector<LODInfo> lodInfos;

        size_t vertexCount = std::visit([](const auto& positionsVec) {
            return positionsVec.size();
            }, mesh->positions);

        // Fill vertex data
        for (size_t i = 0; i < vertexCount; ++i) {
            // Positions
            if (m_MeshLayout.hasPositions) {
                std::visit([&](const auto& positionsVec) {
                    const auto& pos = positionsVec[i];
                    if constexpr (std::is_same_v<std::decay_t<decltype(pos)>, glm::vec3>) {
                        combinedVertexData.insert(combinedVertexData.end(), { pos.x, pos.y, pos.z });
                    }
                    else if constexpr (std::is_same_v<std::decay_t<decltype(pos)>, glm::vec2>) {
                        combinedVertexData.insert(combinedVertexData.end(), { pos.x, pos.y });
                    }
                    }, mesh->positions);
            }

            // Normals
            if (m_MeshLayout.hasNormals && i < mesh->normals.size()) {
                const glm::vec3& normal = mesh->normals[i];
                combinedVertexData.insert(combinedVertexData.end(), { normal.x, normal.y, normal.z });
            }

            // Tangents
            if (m_MeshLayout.hasTangents && i < mesh->tangents.size()) {
                const glm::vec3& tangent = mesh->tangents[i];
                combinedVertexData.insert(combinedVertexData.end(), { tangent.x, tangent.y, tangent.z });
            }

            // Bitangents
            if (m_MeshLayout.hasBitangents && i < mesh->bitangents.size()) {
                const glm::vec3& bitangent = mesh->bitangents[i];
                combinedVertexData.insert(combinedVertexData.end(), { bitangent.x, bitangent.y, bitangent.z });
            }

            // Texture Coordinates
            for (const auto& texType : m_MeshLayout.textureTypes) {
                const auto& uvMap = mesh->uvs.find(texType);
                if (uvMap != mesh->uvs.end() && i < uvMap->second.size()) {
                    const glm::vec2& texCoord = uvMap->second[i];
                    combinedVertexData.insert(combinedVertexData.end(), { texCoord.x, texCoord.y });
                }
                else {
                    combinedVertexData.insert(combinedVertexData.end(), { 0.0f, 0.0f });
                }
            }
        }

        // LODs
        for (const auto& lod : mesh->lods) {
            LODInfo lodInfo;
            lodInfo.indexOffsetInCombinedBuffer = combinedIndices.size();
            lodInfo.indexCount = lod.indexCount;

            for (size_t i = 0; i < lod.indexCount; ++i) {
                GLuint index = mesh->indices[lod.indexOffset + i];
                combinedIndices.push_back(index + baseVertex);
            }

            lodInfos.push_back(lodInfo);
        }

        m_LODInfos.push_back(lodInfos);

        // Use the object's current LOD to determine initial command (default to 0 if not set)
        size_t initialLOD = ro->GetCurrentLOD();
        if (initialLOD >= lodInfos.size()) {
            initialLOD = 0;
        }

        const LODInfo& defaultLODInfo = lodInfos[initialLOD];
        DrawElementsIndirectCommand cmd = {};
        cmd.count = (GLuint)defaultLODInfo.indexCount;
        cmd.instanceCount = 1;
        cmd.firstIndex = (GLuint)defaultLODInfo.indexOffsetInCombinedBuffer;
        cmd.baseVertex = 0;
        cmd.baseInstance = 0;

        m_DrawCommands.push_back(cmd);

        baseVertex += (GLuint)vertexCount;
    }

    // Create/Update VBO
    std::span<const std::byte> vertexSpan(
        reinterpret_cast<const std::byte*>(combinedVertexData.data()),
        combinedVertexData.size() * sizeof(float)
    );

    if (!m_VBO) {
        m_VBO = std::make_shared<VertexBuffer>(vertexSpan, GL_STATIC_DRAW);
    }
    else {
        m_VBO->UpdateData(vertexSpan);
    }

    // Create/Update IBO
    std::span<const GLuint> indexSpan(combinedIndices.data(), combinedIndices.size());
    if (!m_IBO) {
        m_IBO = std::make_shared<IndexBuffer>(indexSpan, GL_STATIC_DRAW);
    }
    else {
        m_IBO->UpdateData(indexSpan);
    }

    // Create/Update IndirectBuffer
    std::span<const std::byte> drawCommandSpan(
        reinterpret_cast<const std::byte*>(m_DrawCommands.data()),
        m_DrawCommands.size() * sizeof(DrawElementsIndirectCommand)
    );

    if (!m_DrawCommandBuffer) {
        m_DrawCommandBuffer = std::make_unique<IndirectBuffer>(drawCommandSpan, GL_DYNAMIC_DRAW);
    }
    else {
        m_DrawCommandBuffer->UpdateData(drawCommandSpan);
    }

    m_VAO->Bind();
    m_VAO->AddBuffer(*m_VBO, vertexBufferLayout);
    m_VAO->SetIndexBuffer(*m_IBO);
    m_VAO->Unbind();
}

void Batch::UpdateLOD(size_t objectIndex, size_t newLOD) {
    if (objectIndex >= m_RenderObjects.size()) {
        Logger::GetLogger()->error("Invalid object index in UpdateLOD.");
        return;
    }

    if (newLOD >= m_LODInfos[objectIndex].size()) {
        Logger::GetLogger()->error("Invalid LOD index in UpdateLOD.");
        return;
    }

    const LODInfo& lodInfo = m_LODInfos[objectIndex][newLOD];
    auto& cmd = m_DrawCommands[objectIndex];
    cmd.count = (GLuint)lodInfo.indexCount;
    cmd.firstIndex = (GLuint)lodInfo.indexOffsetInCombinedBuffer;

    // Update one command in the IndirectBuffer
    std::span<const std::byte> cmdSpan(
        reinterpret_cast<const std::byte*>(&cmd),
        sizeof(DrawElementsIndirectCommand)
    );
    GLintptr offset = objectIndex * sizeof(DrawElementsIndirectCommand);
    m_DrawCommandBuffer->UpdateData(cmdSpan, offset);
}

void Batch::Render() const {
    m_VAO->Bind();
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_DrawCommandBuffer->GetRendererID());
    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, (GLsizei)m_DrawCommands.size(), 0);
    m_VAO->Unbind();
}