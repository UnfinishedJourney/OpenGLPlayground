#include "Mesh.h"

#include <iostream>

void MeshComponent::Bind() const
{
    m_VAO->Bind();
    m_IB->Bind();
}

void MeshComponent::Unbind() const
{
    m_VAO->Unbind();
    m_IB->Unbind();
}

std::shared_ptr<MeshComponent> MeshHelper::CreateMeshComponent(std::shared_ptr<Mesh> mesh, const MeshLayout& layout) const
{

    std::vector<float> vertices;

    const std::vector<glm::vec3>& positions = mesh->positions;
    const std::vector<glm::vec3>& normals = mesh->normals;
    const std::vector<glm::vec3>& tangents = mesh->tangents;
    const std::vector<glm::vec2>& texCoords = mesh->uvs;

    const std::vector<unsigned int>& indices = mesh->indices;

    assert(!layout.hasTangents || !tangents.empty());
    assert(!layout.hasNormals  || !normals.empty());
    assert(!layout.hasUVs      || !texCoords.empty());

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

    std::shared_ptr<VertexBuffer> vb = std::make_shared<VertexBuffer>(&vertices[0], sizeof(float) * vertices.size());

    VertexBufferLayout vb_layout;

    vb_layout.Push<float>(3);
    if (layout.hasNormals)
        vb_layout.Push<float>(3);
    if (layout.hasTangents)
        vb_layout.Push<float>(3);
    if (layout.hasUVs)
        vb_layout.Push<float>(2);

    std::shared_ptr<VertexArray> vao = std::make_shared<VertexArray>();
    vao->AddBuffer(*vb, vb_layout);
    std::shared_ptr<IndexBuffer> ib = std::make_shared<IndexBuffer>(indices.data(), indices.size());

    return std::make_shared<MeshComponent>(MeshComponent( vao, vb, ib, indices.size()));
}

