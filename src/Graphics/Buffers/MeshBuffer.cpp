#include "Graphics/Buffers/MeshBuffer.h"
#include "Utilities/Logger.h"
#include <stdexcept>

MeshBuffer::MeshBuffer(const Mesh& mesh, const MeshLayout& layout) {
    Logger::GetLogger()->info("Initializing MeshBuffer.");

    CreateBuffers(mesh, layout);
}

void MeshBuffer::CreateBuffers(const Mesh& mesh, const MeshLayout& layout) {
    // Validate mesh data according to the layout
    if (layout.hasPositions && mesh.positions.empty()) {
        throw std::runtime_error("Mesh requires positions, but none were found.");
    }
    if (layout.hasNormals && mesh.normals.empty()) {
        throw std::runtime_error("Mesh requires normals, but none were found.");
    }
    if (layout.hasTangents && mesh.tangents.empty()) {
        throw std::runtime_error("Mesh requires tangents, but none were found.");
    }
    if (layout.hasBitangents && mesh.bitangents.empty()) {
        throw std::runtime_error("Mesh requires bitangents, but none were found.");
    }
    for (const auto& texType : layout.textureTypes) {
        if (!mesh.HasTextureCoords(texType)) {
            throw std::runtime_error("Mesh requires texture coordinates, but none were found.");
        }
    }

    // Interleave vertex data
    std::vector<float> vertexData;
    size_t vertexCount = mesh.positions.size();
    size_t stride = 0;

    // Calculate stride
    if (layout.hasPositions) stride += 3;
    if (layout.hasNormals) stride += 3;
    if (layout.hasTangents) stride += 3;
    if (layout.hasBitangents) stride += 3;
    stride += layout.textureTypes.size() * 2;

    vertexData.reserve(vertexCount * stride);

    for (size_t i = 0; i < vertexCount; ++i) {
        if (layout.hasPositions) {
            const glm::vec3& pos = mesh.positions[i];
            vertexData.insert(vertexData.end(), { pos.x, pos.y, pos.z });
        }

        if (layout.hasNormals) {
            const glm::vec3& normal = mesh.normals[i];
            vertexData.insert(vertexData.end(), { normal.x, normal.y, normal.z });
        }

        if (layout.hasTangents) {
            const glm::vec3& tangent = mesh.tangents[i];
            vertexData.insert(vertexData.end(), { tangent.x, tangent.y, tangent.z });
        }

        if (layout.hasBitangents) {
            const glm::vec3& bitangent = mesh.bitangents[i];
            vertexData.insert(vertexData.end(), { bitangent.x, bitangent.y, bitangent.z });
        }

        for (const auto& texType : layout.textureTypes) {
            const glm::vec2& uv = mesh.uvs.at(texType)[i];
            vertexData.insert(vertexData.end(), { uv.x, uv.y });
        }
    }

    m_VertexCount = vertexCount;

    // Create VertexBuffer
    std::span<const std::byte> vertexSpan{
        reinterpret_cast<const std::byte*>(vertexData.data()),
        vertexData.size() * sizeof(float)
    };
    m_VB = std::make_shared<VertexBuffer>(vertexSpan);

    // Create VertexBufferLayout
    VertexBufferLayout vbLayout;
    GLuint attributeIndex = 0;
    if (layout.hasPositions) vbLayout.Push<float>(3, attributeIndex++);
    if (layout.hasNormals) vbLayout.Push<float>(3, attributeIndex++);
    if (layout.hasTangents) vbLayout.Push<float>(3, attributeIndex++);
    if (layout.hasBitangents) vbLayout.Push<float>(3, attributeIndex++);
    for (const auto& texType : layout.textureTypes) {
        vbLayout.Push<float>(2, attributeIndex++);
    }
    m_Layout = vbLayout;

    // Create VertexArray and add buffers
    m_VAO = std::make_shared<VertexArray>();
    m_VAO->AddBuffer(*m_VB, vbLayout);
    m_IB = std::make_shared<IndexBuffer>(std::span<const GLuint>(mesh.indices));
    m_IndexCount = mesh.indices.size();
    m_VAO->SetIndexBuffer(*m_IB);

    Logger::GetLogger()->info("MeshBuffer initialized successfully.");
}

void MeshBuffer::Bind() const {
    m_VAO->Bind();
}

void MeshBuffer::Unbind() const {
    m_VAO->Unbind();
}