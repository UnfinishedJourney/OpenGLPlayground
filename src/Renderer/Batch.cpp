#include "Batch.h"
#include "Utilities/Logger.h"
#include <algorithm>
#include <numeric>

Batch::Batch(const std::string& shaderName, const std::string& materialName, const MeshLayout& meshLayout)
    : m_ShaderName(shaderName),
    m_MaterialName(materialName),
    m_MeshLayout(meshLayout),
    m_VAO(std::make_unique<VertexArray>()) {
}

Batch::~Batch() {
    // Resources are automatically cleaned up
}

void Batch::AddRenderObject(const std::shared_ptr<RenderObject>& renderObject) {
    PROFILE_FUNCTION(Magenta); // Profile the entire function
    m_RenderObjects.push_back(renderObject);
    m_IsDirty = true;
}

const std::vector<std::shared_ptr<RenderObject>>& Batch::GetRenderObjects() const {
    PROFILE_FUNCTION(Green);
    return m_RenderObjects;
}

const std::string& Batch::GetShaderName() const {
    // Assuming this function is trivial, profiling may not be necessary.
    return m_ShaderName;
}

const std::string& Batch::GetMaterialName() const {
    // Assuming this function is trivial, profiling may not be necessary.
    return m_MaterialName;
}

const MeshLayout& Batch::GetMeshLayout() const {
    // Assuming this function is trivial, profiling may not be necessary.
    return m_MeshLayout;
}

void Batch::BuildBatches() {
    PROFILE_FUNCTION(Cyan);
    if (m_RenderObjects.empty()) {
        Logger::GetLogger()->warn("No RenderObjects to batch.");
        return;
    }

    // Build vertex layout
    VertexBufferLayout vertexBufferLayout;
    GLuint attributeIndex = 0;

    size_t positionComponentCount = 3;
    const auto& firstMesh = m_RenderObjects.front()->GetMesh();

    // Directly access positions vector
    if (!firstMesh->positions.empty()) {
        // Assuming all meshes have the same position component count
        // If positions are glm::vec3, then 3 components; glm::vec2 would have 2
        // Here, assuming glm::vec3
        positionComponentCount = 3; // Update accordingly if you have different types
    }

    if (m_MeshLayout.hasPositions) {
        vertexBufferLayout.Push<float>(static_cast<GLuint>(positionComponentCount), attributeIndex++);
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
        {
            PROFILE_BLOCK("Process RenderObject", Blue);
            const auto& mesh = ro->GetMesh();
            std::vector<LODInfo> lodInfos;

            size_t vertexCount = mesh->positions.size();

            // Fill vertex data
            for (size_t i = 0; i < vertexCount; ++i) {
                // Positions
                if (m_MeshLayout.hasPositions) {
                    const glm::vec3& pos = mesh->positions[i];
                    combinedVertexData.insert(combinedVertexData.end(), { pos.x, pos.y, pos.z });
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
                    auto texIt = mesh->uvs.find(texType);
                    if (texIt != mesh->uvs.end() && i < texIt->second.size()) {
                        const glm::vec2& texCoord = texIt->second[i];
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
            cmd.count = static_cast<GLuint>(defaultLODInfo.indexCount);
            cmd.instanceCount = 1;
            cmd.firstIndex = static_cast<GLuint>(defaultLODInfo.indexOffsetInCombinedBuffer);
            cmd.baseVertex = 0;
            cmd.baseInstance = 0;

            m_DrawCommands.push_back(cmd);

            baseVertex += static_cast<GLuint>(vertexCount);
        }
    }

    // Create/Update VBO
    {
        PROFILE_BLOCK("Create/Update VBO", Yellow);
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
    }

    // Create/Update IBO
    {
        PROFILE_BLOCK("Create/Update IBO", Yellow);
        std::span<const GLuint> indexSpan(combinedIndices.data(), combinedIndices.size());
        if (!m_IBO) {
            m_IBO = std::make_shared<IndexBuffer>(indexSpan, GL_STATIC_DRAW);
        }
        else {
            m_IBO->UpdateData(indexSpan);
        }
    }

    // Create/Update IndirectBuffer
    {
        PROFILE_BLOCK("Create/Update IndirectBuffer", Yellow);
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
    }

    // Setup VAO
    {
        PROFILE_BLOCK("Setup VAO", Yellow);
        m_VAO->Bind();
        m_VAO->AddBuffer(*m_VBO, vertexBufferLayout);
        m_VAO->SetIndexBuffer(*m_IBO);
        m_VAO->Unbind();
    }

    m_IsDirty = false;
}

void Batch::Update() {
    PROFILE_FUNCTION(Purple);
    if (m_IsDirty) {
        BuildBatches();
    }
}

void Batch::BuildBatch() {
    PROFILE_FUNCTION(Purple);
    // Rebuild the batch if it's marked dirty
    BuildBatches();
}

void Batch::Render() const {
    PROFILE_FUNCTION(DarkCyan);
    if (m_DrawCommands.empty()) return;

    m_VAO->Bind();
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_DrawCommandBuffer->GetRendererID());
    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(m_DrawCommands.size()), 0);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
    m_VAO->Unbind();
}

void Batch::CullObject(size_t objectIndex) {
    PROFILE_FUNCTION(Red);
    if (objectIndex >= m_DrawCommands.size()) {
        Logger::GetLogger()->error("Invalid object index in CullObject.");
        return;
    }

    // Set the draw count to zero to cull the object
    DrawElementsIndirectCommand& cmd = m_DrawCommands[objectIndex];
    cmd.count = 0;

    // Update the specific command in the IndirectBuffer
    std::span<const std::byte> cmdSpan(
        reinterpret_cast<const std::byte*>(&cmd),
        sizeof(DrawElementsIndirectCommand)
    );
    GLintptr offset = objectIndex * sizeof(DrawElementsIndirectCommand);
    m_DrawCommandBuffer->UpdateData(cmdSpan, offset);
}

void Batch::UpdateLOD(size_t objectIndex, size_t newLOD) {
    PROFILE_FUNCTION(Red);
    if (objectIndex >= m_RenderObjects.size()) {
        Logger::GetLogger()->error("Invalid object index in UpdateLOD.");
        return;
    }

    if (newLOD >= m_LODInfos[objectIndex].size()) {
        Logger::GetLogger()->error("Invalid LOD index in UpdateLOD.");
        return;
    }

    const LODInfo& lodInfo = m_LODInfos[objectIndex][newLOD];
    DrawElementsIndirectCommand& cmd = m_DrawCommands[objectIndex];
    cmd.count = static_cast<GLuint>(lodInfo.indexCount);
    cmd.firstIndex = static_cast<GLuint>(lodInfo.indexOffsetInCombinedBuffer);

    // Update the specific command in the IndirectBuffer
    std::span<const std::byte> cmdSpan(
        reinterpret_cast<const std::byte*>(&cmd),
        sizeof(DrawElementsIndirectCommand)
    );
    GLintptr offset = objectIndex * sizeof(DrawElementsIndirectCommand);
    m_DrawCommandBuffer->UpdateData(cmdSpan, offset);
}