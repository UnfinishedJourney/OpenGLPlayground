#include "Graphics/Buffers/MeshBuffer.h"
#include "Utilities/Logger.h"
#include <stdexcept>
#include <glad/glad.h>
#include <variant>
#include <span> // Include for std::span

MeshBuffer::MeshBuffer(const Mesh& mesh, const MeshLayout& layout)
    : m_MeshLayout(layout), m_VertexCount(0), m_IndexCount(0)
{
    // Check if positions are required and present
    if (layout.hasPositions) {
        bool positionsEmpty = std::visit([](const auto& positionsVec) {
            return positionsVec.empty();
            }, mesh.positions);

        if (positionsEmpty) {
            throw std::runtime_error("Mesh requires positions, but none were found.");
        }
    }

    // Determine vertex count
    m_VertexCount = static_cast<GLuint>(std::visit([](const auto& positionsVec) {
        return positionsVec.size();
        }, mesh.positions));

    // Calculate total components per vertex
    size_t totalComponents = 0;
    if (layout.hasPositions) {
        size_t positionComponentCount = std::visit([](const auto& positionsVec) {
            using VecType = typename std::decay_t<decltype(positionsVec)>::value_type;
            if constexpr (std::is_same_v<VecType, glm::vec3>) {
                return 3;
            }
            else {
                return 2;
            }
            }, mesh.positions);
        totalComponents += positionComponentCount;
    }
    if (layout.hasNormals && !mesh.normals.empty()) {
        totalComponents += 3;
    }
    for (const auto& textureType : layout.textureTypes) {
        totalComponents += 2;
    }
    if (layout.hasTangents && !mesh.tangents.empty()) {
        totalComponents += 3;
    }
    if (layout.hasBitangents && !mesh.bitangents.empty()) {
        totalComponents += 3;
    }

    // Interleave vertex data
    std::vector<float> vertexData;
    vertexData.reserve(m_VertexCount * totalComponents);

    for (size_t i = 0; i < m_VertexCount; ++i) {
        // Positions
        if (layout.hasPositions) {
            std::visit([&](const auto& positionsVec) {
                const auto& pos = positionsVec[i];
                if constexpr (std::is_same_v<std::decay_t<decltype(pos)>, glm::vec3>) {
                    vertexData.insert(vertexData.end(), { pos.x, pos.y, pos.z });
                }
                else if constexpr (std::is_same_v<std::decay_t<decltype(pos)>, glm::vec2>) {
                    vertexData.insert(vertexData.end(), { pos.x, pos.y });
                }
                }, mesh.positions);
        }

        // Normals
        if (layout.hasNormals && !mesh.normals.empty()) {
            const auto& normal = mesh.normals[i];
            vertexData.insert(vertexData.end(), { normal.x, normal.y, normal.z });
        }

        // Texture Coordinates
        for (const auto& textureType : layout.textureTypes) {
            const auto& texCoordsVec = mesh.uvs.at(textureType);
            if (!texCoordsVec.empty()) {
                const auto& texCoord = texCoordsVec[i];
                vertexData.insert(vertexData.end(), { texCoord.x, texCoord.y });
            }
        }

        // Tangents
        if (layout.hasTangents && !mesh.tangents.empty()) {
            const auto& tangent = mesh.tangents[i];
            vertexData.insert(vertexData.end(), { tangent.x, tangent.y, tangent.z });
        }

        // Bitangents
        if (layout.hasBitangents && !mesh.bitangents.empty()) {
            const auto& bitangent = mesh.bitangents[i];
            vertexData.insert(vertexData.end(), { bitangent.x, bitangent.y, bitangent.z });
        }
    }

    // Set up VAO and VBO
    m_VAO = std::make_unique<VertexArray>();

    // Create vertex data span
    std::span<const std::byte> vertexSpan{
        reinterpret_cast<const std::byte*>(vertexData.data()),
        vertexData.size() * sizeof(float)
    };

    m_VBO = std::make_unique<VertexBuffer>(vertexSpan);

    VertexBufferLayout bufferLayout;
    GLuint attributeIndex = 0;

    // Positions
    if (layout.hasPositions) {
        size_t positionComponentCount = std::visit([](const auto& positionsVec) {
            using VecType = typename std::decay_t<decltype(positionsVec)>::value_type;
            if constexpr (std::is_same_v<VecType, glm::vec3>) {
                return 3;
            }
            else {
                return 2;
            }
            }, mesh.positions);

        bufferLayout.Push<float>(static_cast<GLuint>(positionComponentCount), attributeIndex++);
    }

    // Normals
    if (layout.hasNormals && !mesh.normals.empty()) {
        bufferLayout.Push<float>(3, attributeIndex++);
    }

    // Texture Coordinates
    for (const auto& textureType : layout.textureTypes) {
        bufferLayout.Push<float>(2, attributeIndex++);
    }

    // Tangents
    if (layout.hasTangents && !mesh.tangents.empty()) {
        bufferLayout.Push<float>(3, attributeIndex++);
    }

    // Bitangents
    if (layout.hasBitangents && !mesh.bitangents.empty()) {
        bufferLayout.Push<float>(3, attributeIndex++);
    }

    m_VAO->AddBuffer(*m_VBO, bufferLayout);

    // Indices
    if (!mesh.indices.empty()) {
        m_IndexCount = static_cast<GLuint>(mesh.indices.size());

        // Ensure indices are of type GLuint
        std::vector<GLuint> indicesGL(mesh.indices.begin(), mesh.indices.end());
        std::span<const GLuint> indexSpan(indicesGL.data(), indicesGL.size());

        m_IBO = std::make_unique<IndexBuffer>(indexSpan);
    }
    else {
        m_IndexCount = m_VertexCount;
    }
}

void MeshBuffer::Bind() const
{
    m_VAO->Bind();
    if (m_IBO) {
        m_IBO->Bind();
    }
}

void MeshBuffer::Unbind() const
{
    if (m_IBO) {
        m_IBO->Unbind();
    }
    m_VAO->Unbind();
}

void MeshBuffer::Render() const
{
    Bind();
    if (m_IBO) {
        glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, nullptr);
    }
    else {
        glDrawArrays(GL_TRIANGLES, 0, m_VertexCount);
    }
    Unbind();
}