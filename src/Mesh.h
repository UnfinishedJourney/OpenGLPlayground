#pragma once

#include "Utility.h"
#include "glm.hpp"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"

#include <vector>
#include <memory>

class MeshHelper;

class Mesh {
public:
    Mesh() = default;
    virtual ~Mesh() = default;

    
    const std::vector<glm::vec3>& GetPositions() const {
        return m_Positions;
    }

    const std::vector<glm::vec3>& GetNormals() const {
        return m_Normals;
    }

    const std::vector<glm::vec3>& GetBinormals() const {
        return m_Binormals;
    }

    const std::vector<glm::vec3>& GetTangents() const {
        return m_Tangents;
    }

    const std::vector<glm::vec2>& GetUVs() const {
        return m_UVs;
    }

    const std::vector<unsigned int>& GetIndices() const {
        return m_Indices;
    }

    void SetPositions(const std::vector<glm::vec3>& positions) {
        m_Positions = positions;
    }

    void SetNormals(const std::vector<glm::vec3>& normals) {
        m_Normals = normals;
    }

    void SetBinormals(const std::vector<glm::vec3>& binormals) {
        m_Binormals = binormals;
    }

    void SetTangents(const std::vector<glm::vec3>& tangents) {
        m_Tangents = tangents;
    }

    void SetUVs(const std::vector<glm::vec2>& uvs) {
        m_UVs = uvs;
    }

    void SetIndices(const std::vector<unsigned int>& indices) {
        m_Indices = indices;
    }

    friend MeshHelper;

protected:
    std::vector<glm::vec3> m_Positions;
    std::vector<glm::vec3> m_Normals;
    std::vector<glm::vec3> m_Binormals;
    std::vector<glm::vec3> m_Tangents;
    std::vector<glm::vec2> m_UVs;
    std::vector<unsigned int> m_Indices;
};

class MeshComponent {
public:
    MeshComponent(std::shared_ptr<VertexArray> vao, std::shared_ptr<VertexBuffer> vb, std::shared_ptr<IndexBuffer> ib, size_t nVerts)
        : m_VAO(vao), m_VB(std::move(vb)), m_IB(std::move(ib)), m_NVerts(nVerts)
    {}

    void Bind() const;
    void Unbind() const;
    size_t GetNVerts() const
    {
        return m_NVerts;
    }

    friend MeshHelper;

private:
    std::shared_ptr<VertexArray> m_VAO;
    std::shared_ptr<VertexBuffer> m_VB;
    std::shared_ptr<IndexBuffer> m_IB;
    size_t m_NVerts;
};

struct MeshLayout
{
    bool hasPositions;
    bool hasNormals;
    bool hasBinormals;
    bool hasTangents;
    bool hasUVs;
};

class MeshHelper {
public:
    std::shared_ptr<MeshComponent> CreateMeshComponent(std::shared_ptr<Mesh> mesh, const MeshLayout& layout) const;
};