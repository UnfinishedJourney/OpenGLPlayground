#include "MeshBuffer.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <stdexcept>
#include <vector>
#include <span>
#include <glad/glad.h>

namespace Graphics {

    MeshBuffer::MeshBuffer(const Mesh& mesh, const MeshLayout& layout)
        : m_MeshLayout(layout)
    {
        // Validate that required position data exists.
        if (m_MeshLayout.hasPositions && mesh.positions.empty()) {
            Logger::GetLogger()->error("Mesh requires positions, but none were provided.");
            throw std::runtime_error("Mesh requires positions, but none were provided.");
        }

        m_VertexCount = static_cast<GLuint>(mesh.positions.size());

        // Calculate total float components per vertex.
        size_t totalComponents = 0;
        if (m_MeshLayout.hasPositions)  totalComponents += 3; // (x, y, z)
        if (m_MeshLayout.hasNormals && !mesh.normals.empty()) totalComponents += 3; // (nx, ny, nz)
        for (const auto& textureType : m_MeshLayout.textureTypes) {
            const auto& texCoordsVec = mesh.uvs.at(textureType);
            if (!texCoordsVec.empty()) totalComponents += 2; // (u, v)
        }
        if (m_MeshLayout.hasTangents && !mesh.tangents.empty()) totalComponents += 3;
        if (m_MeshLayout.hasBitangents && !mesh.bitangents.empty()) totalComponents += 3;

        // Interleave vertex data.
        std::vector<float> vertexData;
        vertexData.reserve(m_VertexCount * totalComponents);

        for (size_t i = 0; i < m_VertexCount; ++i) {
            if (m_MeshLayout.hasPositions) {
                const auto& pos = mesh.positions[i];
                vertexData.insert(vertexData.end(), { pos.x, pos.y, pos.z });
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

        // Create VAO and VBO.
        m_VAO = std::make_unique<VertexArray>();
        m_VAO->Bind();

        std::span<const std::byte> vertexSpan{
            reinterpret_cast<const std::byte*>(vertexData.data()),
            vertexData.size() * sizeof(float)
        };
        m_VBO = std::make_unique<VertexBuffer>(vertexSpan);

        // Build the layout for the VBO.
        VertexBufferLayout bufferLayout;
        GLuint attributeIndex = 0;
        if (m_MeshLayout.hasPositions) {
            bufferLayout.Push<float>(3, attributeIndex++);
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

        // Optionally create IBO if indices are provided.
        if (!mesh.indices.empty()) {
            m_IndexCount = static_cast<GLuint>(mesh.indices.size());
            m_HasIndices = true;

            std::vector<GLuint> indicesGL(mesh.indices.begin(), mesh.indices.end());
            std::span<const GLuint> indexSpan(indicesGL.data(), indicesGL.size());
            m_IBO = std::make_unique<IndexBuffer>(indexSpan);
        }
        else {
            m_IndexCount = m_VertexCount;
            m_HasIndices = false;
        }

        m_VAO->Unbind();
    }

    void MeshBuffer::Bind() const {
        m_VAO->Bind();
        if (m_HasIndices && m_IBO) {
            m_IBO->Bind();
        }
    }

    void MeshBuffer::Unbind() const {
        if (m_HasIndices && m_IBO) {
            m_IBO->Unbind();
        }
        m_VAO->Unbind();
    }

    void MeshBuffer::Render() const {
        Bind();
        if (m_HasIndices) {
            GLCall(glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, nullptr));
        }
        else {
            GLCall(glDrawArrays(GL_TRIANGLES, 0, m_VertexCount));
        }
        Unbind();
    }

} // namespace Graphics