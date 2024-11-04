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
    if (m_MeshLayout.hasPositions) {
        vertexBufferLayout.Push<float>(3); // Positions
    }
    if (m_MeshLayout.hasNormals) {
        vertexBufferLayout.Push<float>(3); // Normals
    }
    if (m_MeshLayout.hasTangents) {
        vertexBufferLayout.Push<float>(3); // Tangents
    }
    if (m_MeshLayout.hasBitangents) {
        vertexBufferLayout.Push<float>(3); // Bitangents
    }
    for (const auto& texType : m_MeshLayout.textureTypes) {
        vertexBufferLayout.Push<float>(2); // Texture Coordinates
    }

    // Combine vertex data
    std::vector<GLfloat> vertexData;
    std::vector<GLuint> indices;
    size_t vertexOffset = 0;

    for (const auto& ro : m_RenderObjects) {
        auto mesh = ro->GetMesh();
        auto N = mesh->GetVertexCount();
        mesh->GetIndexCount();

        for (int i = 0; i < N; i++) {
            if (m_MeshLayout.hasPositions) {
                const glm::vec3& pos = mesh->positions[i];
                vertexData.insert(vertexData.end(), { pos.x, pos.y, pos.z });
            }

            if (m_MeshLayout.hasNormals) {
                const glm::vec3& normal = mesh->normals[i];
                vertexData.insert(vertexData.end(), { normal.x, normal.y, normal.z });
            }

            if (m_MeshLayout.hasTangents) {
                const glm::vec3& tangent = mesh->tangents[i];
                vertexData.insert(vertexData.end(), { tangent.x, tangent.y, tangent.z });
            }

            if (m_MeshLayout.hasBitangents) {
                const glm::vec3& bitangent = mesh->bitangents[i];
                vertexData.insert(vertexData.end(), { bitangent.x, bitangent.y, bitangent.z });
            }

            for (const auto& texType : m_MeshLayout.textureTypes) {
                const glm::vec2& uv = mesh->uvs.at(texType)[i];
                vertexData.insert(vertexData.end(), { uv.x, uv.y });
            }
        }

        for (const auto& index : mesh->indices) {
            indices.push_back(index + vertexOffset);
        }

        vertexOffset = N;
    }

    m_IndexCount = static_cast<GLsizei>(indices.size());

    // Create buffers
    std::span<const std::byte> vertexSpan{
    reinterpret_cast<const std::byte*>(vertexData.data()),
    vertexData.size() * sizeof(GLfloat)
    };

    m_VBO = std::make_unique<VertexBuffer>(vertexSpan);
    m_IBO = std::make_unique<IndexBuffer>(std::span<const GLuint>(indices.data(), indices.size()));

    // Set up VertexArray
    m_VAO->Bind();
    m_VAO->AddBuffer(*m_VBO, vertexBufferLayout);
    m_IBO->Bind();
    m_VAO->Unbind();
}

//void Batch::BuildBatch()
//{
//    if (m_RenderObjects.empty()) {
//        Logger::GetLogger()->warn("No RenderObjects to batch.");
//        return;
//    }
//
//    // Build VertexBufferLayout from MeshLayout
//    VertexBufferLayout vertexBufferLayout;
//    if (m_MeshLayout.hasPositions) {
//        vertexBufferLayout.Push<float>(3); // Positions
//    }
//    if (m_MeshLayout.hasNormals) {
//        vertexBufferLayout.Push<float>(3); // Normals
//    }
//    if (m_MeshLayout.hasTangents) {
//        vertexBufferLayout.Push<float>(3); // Tangents
//    }
//    if (m_MeshLayout.hasBitangents) {
//        vertexBufferLayout.Push<float>(3); // Bitangents
//    }
//    for (const auto& texType : m_MeshLayout.textureTypes) {
//        vertexBufferLayout.Push<float>(2); // Texture Coordinates
//    }
//
//    // Combine vertex data
//    std::vector<std::byte> combinedVertexData;
//    std::vector<GLuint> combinedIndices;
//    GLuint vertexOffset = 0;
//
//    for (const auto& ro : m_RenderObjects) {
//        const auto& mesh = ro->GetMesh();
//
//        // Interleave vertex data
//        size_t vertexCount = mesh->positions.size();
//        for (size_t i = 0; i < vertexCount; ++i) {
//            // Positions
//            if (m_MeshLayout.hasPositions && i < mesh->positions.size()) {
//                const glm::vec3& position = mesh->positions[i];
//                combinedVertexData.insert(combinedVertexData.end(), reinterpret_cast<const std::byte*>(&position), reinterpret_cast<const std::byte*>(&position) + sizeof(glm::vec3));
//            }
//
//            // Normals
//            if (m_MeshLayout.hasNormals && i < mesh->normals.size()) {
//                const glm::vec3& normal = mesh->normals[i];
//                combinedVertexData.insert(combinedVertexData.end(), reinterpret_cast<const std::byte*>(&normal), reinterpret_cast<const std::byte*>(&normal) + sizeof(glm::vec3));
//            }
//
//            // Tangents
//            if (m_MeshLayout.hasTangents && i < mesh->tangents.size()) {
//                const glm::vec3& tangent = mesh->tangents[i];
//                combinedVertexData.insert(combinedVertexData.end(), reinterpret_cast<const std::byte*>(&tangent), reinterpret_cast<const std::byte*>(&tangent) + sizeof(glm::vec3));
//            }
//
//            // Bitangents
//            if (m_MeshLayout.hasBitangents && i < mesh->bitangents.size()) {
//                const glm::vec3& bitangent = mesh->bitangents[i];
//                combinedVertexData.insert(combinedVertexData.end(), reinterpret_cast<const std::byte*>(&bitangent), reinterpret_cast<const std::byte*>(&bitangent) + sizeof(glm::vec3));
//            }
//
//            // Texture Coordinates
//            for (const auto& texType : m_MeshLayout.textureTypes) {
//                const auto& uvMap = mesh->uvs.find(texType);
//                if (uvMap != mesh->uvs.end() && i < uvMap->second.size()) {
//                    const glm::vec2& texCoord = uvMap->second[i];
//                    combinedVertexData.insert(combinedVertexData.end(), reinterpret_cast<const std::byte*>(&texCoord), reinterpret_cast<const std::byte*>(&texCoord) + sizeof(glm::vec2));
//                }
//                else {
//                    // Insert default texCoord if not available
//                    glm::vec2 defaultTexCoord(0.0f, 0.0f);
//                    combinedVertexData.insert(combinedVertexData.end(), reinterpret_cast<const std::byte*>(&defaultTexCoord), reinterpret_cast<const std::byte*>(&defaultTexCoord) + sizeof(glm::vec2));
//                }
//            }
//        }
//
//        // Indices
//        for (const auto& index : mesh->indices) {
//            combinedIndices.push_back(index + vertexOffset);
//        }
//
//        vertexOffset += static_cast<GLuint>(vertexCount);
//    }
//
//    m_IndexCount = static_cast<GLsizei>(combinedIndices.size());
//
//    // Create buffers
//    m_VBO = std::make_unique<VertexBuffer>(std::span<const std::byte>(combinedVertexData.data(), combinedVertexData.size()));
//    m_IBO = std::make_unique<IndexBuffer>(std::span<const GLuint>(combinedIndices.data(), combinedIndices.size()));
//
//    // Set up VertexArray
//    m_VAO->Bind();
//    m_VAO->AddBuffer(*m_VBO, vertexBufferLayout);
//    m_IBO->Bind();
//    m_VAO->Unbind();
//}

void Batch::Render() const
{
    m_VAO->Bind();
    glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, nullptr);
    m_VAO->Unbind();
}