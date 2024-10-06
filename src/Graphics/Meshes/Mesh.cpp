#include "Graphics/Meshes/Mesh.h"

#include <iostream>


MeshBuffer::MeshBuffer(std::shared_ptr<Mesh> mesh, const MeshLayout& mLayout)
{
    std::vector<float> vertices;

    const std::vector<glm::vec3>& positions = mesh->positions;
    const std::vector<glm::vec3>& normals = mesh->normals;
    const std::vector<glm::vec3>& tangents = mesh->tangents;
    const std::vector<glm::vec3>& bitangents = mesh->bitangents;
    const std::unordered_map<TextureType, std::vector<glm::vec2>>& uvs = mesh->uvs;

    const std::vector<unsigned int>& indices = mesh->indices;

    assert(mLayout.hasPositions && "MeshLayout must have positions");
    if (mLayout.hasNormals)
        assert(!normals.empty() && "MeshLayout indicates presence of normals, but mesh has none");
    if (mLayout.hasTangents)
        assert(!tangents.empty() && "MeshLayout indicates presence of tangents, but mesh has none");
    if (mLayout.hasBitangents)
        assert(!bitangents.empty() && "MeshLayout indicates presence of bitangents, but mesh has none");
    for (const auto& texType : mLayout.textureTypes)
    {
        assert(uvs.find(texType) != uvs.end() && "MeshLayout indicates presence of TextureType UVs, but mesh uvs map does not contain it");
        assert(uvs.at(texType).size() == positions.size() && "UVs size mismatch with positions");
    }

    for (size_t i = 0; i < positions.size(); i++) {
        vertices.push_back(positions[i].x);
        vertices.push_back(positions[i].y);
        vertices.push_back(positions[i].z);

        if (mLayout.hasNormals) {
            vertices.push_back(normals[i].x);
            vertices.push_back(normals[i].y);
            vertices.push_back(normals[i].z);
        }

        if (mLayout.hasTangents) {
            vertices.push_back(tangents[i].x);
            vertices.push_back(tangents[i].y);
            vertices.push_back(tangents[i].z);
        }

        if (mLayout.hasBitangents) {
            vertices.push_back(bitangents[i].x);
            vertices.push_back(bitangents[i].y);
            vertices.push_back(bitangents[i].z);
        }

        for (const auto& texType : mLayout.textureTypes)
        {
            const glm::vec2& uv = uvs.at(texType)[i];
            vertices.push_back(uv.x);
            vertices.push_back(uv.y);
        }
    }

    m_VB = std::make_unique<VertexBuffer>(&vertices[0], sizeof(float) * vertices.size());

    VertexBufferLayout vbLayout;

    vbLayout.Push<float>(3);

    if (mLayout.hasNormals)
        vbLayout.Push<float>(3);

    if (mLayout.hasTangents)
        vbLayout.Push<float>(3);

    if (mLayout.hasBitangents)
        vbLayout.Push<float>(3);

    for (const auto& texType : mLayout.textureTypes)
    {
        vbLayout.Push<float>(2);
    }

    m_VAO = std::make_unique<VertexArray>();
    m_VAO->AddBuffer(*m_VB, vbLayout);

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
