#include "Mesh.h"

#include <iostream>

MeshBuffer::MeshBuffer(std::shared_ptr<Mesh> mesh, const MeshLayout& layout)
{
    std::vector<float> vertices;

    const std::vector<glm::vec3>& positions = mesh->positions;
    const std::vector<glm::vec3>& normals = mesh->normals;
    const std::vector<glm::vec3>& tangents = mesh->tangents;
    const std::vector<glm::vec2>& texCoords = mesh->uvs;

    const std::vector<unsigned int>& indices = mesh->indices;

    assert(!layout.hasTangents || !tangents.empty());
    assert(!layout.hasNormals || !normals.empty());
    assert(!layout.hasUVs || !texCoords.empty());

    for (size_t i = 0; i < positions.size(); i++) {
        vertices.push_back(positions[i].x);
        vertices.push_back(positions[i].y);
        vertices.push_back(positions[i].z);

        if (layout.hasNormals) {
            vertices.push_back(normals[i].x);
            vertices.push_back(normals[i].y);
            vertices.push_back(normals[i].z);
        }

        if (layout.hasTangents) {
            vertices.push_back(tangents[i].x);
            vertices.push_back(tangents[i].y);
            vertices.push_back(tangents[i].z);
        }

        if (layout.hasUVs) {
            vertices.push_back(texCoords[i].x);
            vertices.push_back(texCoords[i].y);
        }
    }

    m_VB = std::make_unique<VertexBuffer>(&vertices[0], sizeof(float) * vertices.size());

    VertexBufferLayout vb_layout;

    vb_layout.Push<float>(3);
    if (layout.hasNormals)
        vb_layout.Push<float>(3);
    if (layout.hasTangents)
        vb_layout.Push<float>(3);
    if (layout.hasUVs)
        vb_layout.Push<float>(2);

    m_VAO = std::make_unique<VertexArray>();
    m_VAO->AddBuffer(*m_VB, vb_layout);
    m_IB = std::make_unique<IndexBuffer>(indices.data(), indices.size());
    m_NVerts = indices.size();
}

void MeshBuffer::Bind() const
{
    m_VAO->Bind();
    m_IB->Bind();
}

void MeshBuffer::Unbind() const
{
    m_VAO->Unbind();
    m_IB->Unbind();
}
