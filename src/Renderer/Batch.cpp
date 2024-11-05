#include "Renderer/Batch.h"
#include "Utilities/Logger.h"
#include <glad/glad.h>

Batch::Batch(const std::string& shaderName, const std::string& materialName, const MeshLayout& meshLayout)
    : m_ShaderName(shaderName),
    m_MaterialName(materialName),
    m_MeshLayout(meshLayout)
{
    m_VAO = std::make_unique<VertexArray>();
}

Batch::~Batch()
{
    // Resources are automatically cleaned up
}

void Batch::AddRenderObject(const std::shared_ptr<RenderObject>& renderObject)
{
    m_RenderObjects.push_back(renderObject);
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
        vertexBufferLayout.Push<float>(3, attributeIndex++); // Positions
    }
    if (m_MeshLayout.hasNormals) {
        vertexBufferLayout.Push<float>(3, attributeIndex++); // Normals
    }
    if (m_MeshLayout.hasTangents) {
        vertexBufferLayout.Push<float>(3, attributeIndex++); // Tangents
    }
    if (m_MeshLayout.hasBitangents) {
        vertexBufferLayout.Push<float>(3, attributeIndex++); // Bitangents
    }
    for (const auto& texType : m_MeshLayout.textureTypes) {
        vertexBufferLayout.Push<float>(2, attributeIndex++); // Texture Coordinates
    }

    // Combine vertex data
    std::vector<float> combinedVertexData;
    std::vector<GLuint> combinedIndices;
    GLuint vertexOffset = 0;
    size_t totalVertexCount = 0;
    size_t totalIndexCount = 0;

    // Pre-calculate total sizes to reserve memory
    for (const auto& ro : m_RenderObjects) {
        totalVertexCount += ro->GetMesh()->GetVertexCount();
        totalIndexCount += ro->GetMesh()->GetIndexCount();
    }

    combinedVertexData.reserve(totalVertexCount * attributeIndex);
    combinedIndices.reserve(totalIndexCount);

    for (const auto& ro : m_RenderObjects) {
        const auto& mesh = ro->GetMesh();

        // Interleave vertex data
        size_t vertexCount = mesh->positions.size();
        for (size_t i = 0; i < vertexCount; ++i) {
            if (m_MeshLayout.hasPositions) {
                const glm::vec3& position = mesh->positions[i];
                combinedVertexData.insert(combinedVertexData.end(), { position.x, position.y, position.z });
            }

            if (m_MeshLayout.hasNormals) {
                const glm::vec3& normal = mesh->normals[i];
                combinedVertexData.insert(combinedVertexData.end(), { normal.x, normal.y, normal.z });
            }

            if (m_MeshLayout.hasTangents) {
                const glm::vec3& tangent = mesh->tangents[i];
                combinedVertexData.insert(combinedVertexData.end(), { tangent.x, tangent.y, tangent.z });
            }

            if (m_MeshLayout.hasBitangents) {
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

    // Create buffers
    std::span<const std::byte> vertexSpan{
        reinterpret_cast<const std::byte*>(combinedVertexData.data()),
        combinedVertexData.size() * sizeof(float)
    };
    m_VBO = std::make_unique<VertexBuffer>(vertexSpan);
    m_IBO = std::make_unique<IndexBuffer>(std::span<const GLuint>(combinedIndices.data(), combinedIndices.size()));

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
}