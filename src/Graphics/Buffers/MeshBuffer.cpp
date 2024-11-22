#include "Graphics/Buffers/MeshBuffer.h"
#include "Utilities/Logger.h"
#include <stdexcept>
#include <glad/glad.h>
#include <variant>
#include <span> 

MeshBuffer::MeshBuffer(const Mesh& mesh, const MeshLayout& layout)
    : m_MeshLayout(layout)
{
    if (m_MeshLayout.hasPositions) {
        bool positionsEmpty = std::visit([](const auto& positionsVec) {
            return positionsVec.empty();
            }, mesh.positions);

        if (positionsEmpty) {
            Logger::GetLogger()->error("Mesh requires positions, but none were found.");
            throw std::runtime_error("Mesh requires positions, but none were found.");
        }
    }

    m_VertexCount = static_cast<GLuint>(std::visit([](const auto& positionsVec) {
        return positionsVec.size();
        }, mesh.positions));

    size_t totalComponents = 0;
    if (m_MeshLayout.hasPositions) {
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
    if (m_MeshLayout.hasNormals && !mesh.normals.empty()) {
        totalComponents += 3;
    }
    for (const auto& textureType : m_MeshLayout.textureTypes) {
        const auto& texCoordsVec = mesh.uvs.at(textureType);
        if (!texCoordsVec.empty()) {
            totalComponents += 2;
        }
    }
    if (m_MeshLayout.hasTangents && !mesh.tangents.empty()) {
        totalComponents += 3;
    }
    if (m_MeshLayout.hasBitangents && !mesh.bitangents.empty()) {
        totalComponents += 3;
    }

    std::vector<float> vertexData;
    vertexData.reserve(m_VertexCount * totalComponents);

    for (size_t i = 0; i < m_VertexCount; ++i) {
        if (m_MeshLayout.hasPositions) {
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

        if (m_MeshLayout.hasNormals && !mesh.normals.empty()) {
            const auto& normal = mesh.normals[i];
            vertexData.insert(vertexData.end(), { normal.x, normal.y, normal.z });
        }

        for (const auto& textureType : m_MeshLayout.textureTypes) {
            const auto& texCoordsVec = mesh.uvs.at(textureType);
            if (!texCoordsVec.empty()) {
                const auto& texCoord = texCoordsVec[i];
                vertexData.insert(vertexData.end(), { texCoord.x, texCoord.y });
            }
        }

        if (m_MeshLayout.hasTangents && !mesh.tangents.empty()) {
            const auto& tangent = mesh.tangents[i];
            vertexData.insert(vertexData.end(), { tangent.x, tangent.y, tangent.z });
        }

        if (m_MeshLayout.hasBitangents && !mesh.bitangents.empty()) {
            const auto& bitangent = mesh.bitangents[i];
            vertexData.insert(vertexData.end(), { bitangent.x, bitangent.y, bitangent.z });
        }
    }

    m_VAO = std::make_unique<VertexArray>();

    std::span<const std::byte> vertexSpan{
        reinterpret_cast<const std::byte*>(vertexData.data()),
        vertexData.size() * sizeof(float)
    };

    m_VBO = std::make_unique<VertexBuffer>(vertexSpan);

    VertexBufferLayout bufferLayout;
    GLuint attributeIndex = 0;

    if (m_MeshLayout.hasPositions) {
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

    if (m_MeshLayout.hasNormals && !mesh.normals.empty()) {
        bufferLayout.Push<float>(3, attributeIndex++);
    }

    for (const auto& textureType : m_MeshLayout.textureTypes) {
        const auto& texCoordsVec = mesh.uvs.at(textureType);
        if (!texCoordsVec.empty()) {
            bufferLayout.Push<float>(2, attributeIndex++);
        }
    }

    if (m_MeshLayout.hasTangents && !mesh.tangents.empty()) {
        bufferLayout.Push<float>(3, attributeIndex++);
    }

    if (m_MeshLayout.hasBitangents && !mesh.bitangents.empty()) {
        bufferLayout.Push<float>(3, attributeIndex++);
    }

    m_VAO->AddBuffer(*m_VBO, bufferLayout);

    if (!mesh.indices.empty()) {
        m_IndexCount = static_cast<GLuint>(mesh.indices.size());

        std::vector<GLuint> indicesGL(mesh.indices.begin(), mesh.indices.end());
        std::span<const GLuint> indexSpan(indicesGL.data(), indicesGL.size());

        m_IBO = std::make_unique<IndexBuffer>(indexSpan);
        m_BHasIndices = true;
    }
    else {
        m_IndexCount = m_VertexCount;
        m_BHasIndices = false;
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
        GLCall(glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, nullptr));
    }
    else {
        GLCall(glDrawArrays(GL_TRIANGLES, 0, m_VertexCount));
    }
    Unbind();
}