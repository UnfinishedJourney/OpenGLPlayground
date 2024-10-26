#include "Graphics/Buffers/MeshBuffer.h"
#include "Utilities/Logger.h"
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <stdexcept>

MeshBuffer::MeshBuffer(const Mesh& mesh, const MeshLayout& layout) {
    Logger::GetLogger()->info("Initializing MeshBuffer with provided mesh and layout.");

    // Validate mesh data according to the layout
    if (layout.hasPositions && mesh.positions.empty()) {
        throw std::runtime_error("MeshLayout requires positions, but mesh has none.");
    }
    if (layout.hasNormals && mesh.normals.empty()) {
        throw std::runtime_error("MeshLayout requires normals, but mesh has none.");
    }
    if (layout.hasTangents && mesh.tangents.empty()) {
        throw std::runtime_error("MeshLayout requires tangents, but mesh has none.");
    }
    if (layout.hasBitangents && mesh.bitangents.empty()) {
        throw std::runtime_error("MeshLayout requires bitangents, but mesh has none.");
    }
    for (const auto& texType : layout.textureTypes) {
        if (mesh.uvs.find(texType) == mesh.uvs.end() || mesh.uvs.at(texType).size() != mesh.positions.size()) {
            throw std::runtime_error("MeshLayout requires UVs for a texture type, but mesh UVs are missing or mismatched.");
        }
    }

    // Interleave vertex data according to the layout
    std::vector<GLfloat> vertexData;
    size_t vertexCount = mesh.positions.size();

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

    Logger::GetLogger()->info("Creating VertexBuffer.");
    std::span<const std::byte> vertexSpan{
        reinterpret_cast<const std::byte*>(vertexData.data()),
        vertexData.size() * sizeof(GLfloat)
    };

    m_VB = std::make_shared<VertexBuffer>(vertexSpan);

    Logger::GetLogger()->info("Setting up VertexBufferLayout.");
    VertexBufferLayout vbLayout;
    GLuint attributeIndex = 0;
    GLuint offset = 0;

    if (layout.hasPositions) {
        vbLayout.Push<GLfloat>(3);
        Logger::GetLogger()->debug("Added position attribute to layout.");
        offset += 3 * sizeof(GLfloat);
        ++attributeIndex;
    }

    if (layout.hasNormals) {
        vbLayout.Push<GLfloat>(3);
        Logger::GetLogger()->debug("Added normal attribute to layout.");
        offset += 3 * sizeof(GLfloat);
        ++attributeIndex;
    }

    if (layout.hasTangents) {
        vbLayout.Push<GLfloat>(3);
        Logger::GetLogger()->debug("Added tangent attribute to layout.");
        offset += 3 * sizeof(GLfloat);
        ++attributeIndex;
    }

    if (layout.hasBitangents) {
        vbLayout.Push<GLfloat>(3);
        Logger::GetLogger()->debug("Added bitangent attribute to layout.");
        offset += 3 * sizeof(GLfloat);
        ++attributeIndex;
    }

    for (const auto& texType [[maybe_unused]] : layout.textureTypes) {
        vbLayout.Push<GLfloat>(2);
        Logger::GetLogger()->debug("Added UV attribute to layout.");
        offset += 2 * sizeof(GLfloat);
        ++attributeIndex;
    }

    Logger::GetLogger()->info("Creating VertexArray and adding VertexBuffer.");
    m_VAO = std::make_shared<VertexArray>();
    m_VAO->AddBuffer(*m_VB.get(), vbLayout);

    Logger::GetLogger()->info("Creating IndexBuffer.");
    m_IB = std::make_shared<IndexBuffer>(std::span<const GLuint>(mesh.indices));
    m_IndexCount = mesh.indices.size();

    Logger::GetLogger()->info("MeshBuffer initialization complete.");
}

void MeshBuffer::Bind() const {
    m_VAO->Bind();
    m_IB->Bind();
}

void MeshBuffer::Unbind() const {
    m_VAO->Unbind();
    m_IB->Unbind();
}