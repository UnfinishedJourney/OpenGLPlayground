#include "Renderer/Batch.h"
#include "Utilities/Logger.h"
#include <glad/glad.h>
#include <variant>

Batch::Batch(const std::string& shaderName, const std::string& materialName, const MeshLayout& meshLayout)
    : m_ShaderName(shaderName),
    m_MaterialName(materialName),
    m_MeshLayout(meshLayout),
    m_VAO(std::make_unique<VertexArray>())
{
}

Batch::~Batch()
{
    // Resources are automatically cleaned up
}

void Batch::AddRenderObject(const std::shared_ptr<RenderObject>& renderObject)
{
    m_RenderObjects.push_back(renderObject);
    m_IsDirty = true;
}

void Batch::Update()
{
    if (!m_IsDirty) return;

    BuildBatch();
    m_IsDirty = false;
}

void Batch::BuildBatch()
{
    if (m_RenderObjects.empty()) {
        Logger::GetLogger()->warn("No RenderObjects to batch.");
        return;
    }

    // Build VertexBufferLayout from MeshLayout
    VertexBufferLayout vertexBufferLayout;
    GLuint attributeIndex = 0;
    if (m_MeshLayout.hasPositions) {
        size_t positionComponentCount = 3; // Default to vec3
        const auto& firstMesh = m_RenderObjects.front()->GetMesh();
        std::visit([&](const auto& positionsVec) {
            using VecType = typename std::decay_t<decltype(positionsVec)>::value_type;
            if constexpr (std::is_same_v<VecType, glm::vec2>) {
                positionComponentCount = 2;
            }
            }, firstMesh->positions);
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

    // Combine vertex data
    std::vector<float> combinedVertexData;
    std::vector<GLuint> combinedIndices;
    GLuint vertexOffset = 0;
    size_t totalVertexCount = 0;
    size_t totalIndexCount = 0;

    // Pre-calculate total sizes to reserve memory
    for (const auto& ro : m_RenderObjects) {
        size_t vertexCount = std::visit([](const auto& positionsVec) {
            return positionsVec.size();
            }, ro->GetMesh()->positions);
        totalVertexCount += vertexCount;
        totalIndexCount += ro->GetMesh()->indices.size();
    }

    combinedVertexData.reserve(totalVertexCount * (vertexBufferLayout.GetStride() / sizeof(float)));
    combinedIndices.reserve(totalIndexCount);

    for (const auto& ro : m_RenderObjects) {
        const auto& mesh = ro->GetMesh();

        size_t vertexCount = std::visit([](const auto& positionsVec) {
            return positionsVec.size();
            }, mesh->positions);

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
                    // Insert default texCoord if not available
                    combinedVertexData.insert(combinedVertexData.end(), { 0.0f, 0.0f });
                }
            }
        }

        // Indices
        for (const auto& index : mesh->indices) {
            combinedIndices.push_back(index + vertexOffset);
        }

        vertexOffset += static_cast<GLuint>(vertexCount);
    }

    m_IndexCount = static_cast<GLsizei>(combinedIndices.size());

    // Create or update VertexBuffer
    std::span<const std::byte> vertexSpan{
        reinterpret_cast<const std::byte*>(combinedVertexData.data()),
        combinedVertexData.size() * sizeof(float)
    };
    if (!m_VBO) {
        m_VBO = std::make_unique<VertexBuffer>(vertexSpan, GL_DYNAMIC_DRAW);
    }
    else {
        m_VBO->UpdateData(vertexSpan);
    }

    // Create or update IndexBuffer
    std::span<const GLuint> indexSpan(combinedIndices.data(), combinedIndices.size());
    if (!m_IBO) {
        m_IBO = std::make_unique<IndexBuffer>(indexSpan, GL_DYNAMIC_DRAW);
    }
    else {
        m_IBO->UpdateData(indexSpan);
    }

    // Set up VertexArray
    m_VAO->Bind();
    m_VAO->AddBuffer(*m_VBO, vertexBufferLayout);
    m_VAO->SetIndexBuffer(*m_IBO);
    m_VAO->Unbind();
}

void Batch::Render() const
{
    m_VAO->Bind();
    GLCall(glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, nullptr));
    m_VAO->Unbind();
}