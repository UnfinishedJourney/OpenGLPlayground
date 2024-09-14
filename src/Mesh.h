#pragma once

#include "Utility.h"
#include "glm.hpp"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"

#include <vector>
#include <memory>

class MeshHelper;

struct Mesh
{
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> binormals;
    std::vector<glm::vec3> tangents;
    std::vector<glm::vec2> uvs;
    std::vector<unsigned int> indices;
};

struct MeshLayout
{
    bool hasPositions;
    bool hasNormals;
    bool hasBinormals;
    bool hasTangents;
    bool hasUVs;

    bool operator==(const MeshLayout& other) const {
        return hasPositions == other.hasPositions &&
            hasNormals == other.hasNormals &&
            hasTangents == other.hasTangents &&
            hasBinormals == other.hasBinormals &&
            hasUVs == other.hasUVs;
    }
};

class MeshBuffer {
public:
    MeshBuffer(std::unique_ptr<VertexArray> vao, std::unique_ptr<VertexBuffer> vb, std::unique_ptr<IndexBuffer> ib, size_t nVerts)
        : m_VAO(std::move(vao)), m_VB(std::move(vb)), m_IB(std::move(ib)), m_NVerts(nVerts)
    {}

    MeshBuffer(std::shared_ptr<Mesh> mesh, const MeshLayout& layout);

    void Bind() const;
    void Unbind() const;
    size_t GetNVerts() const
    {
        return m_NVerts;
    }

    friend MeshHelper;

private:
    std::unique_ptr<VertexArray> m_VAO;
    std::unique_ptr<VertexBuffer> m_VB;
    std::unique_ptr<IndexBuffer> m_IB;
    size_t m_NVerts;
};

namespace std {
    template <>
    struct hash<MeshLayout> {
        std::size_t operator()(const MeshLayout& layout) const {
            return (std::hash<bool>()(layout.hasPositions) ^
                (std::hash<bool>()(layout.hasNormals) << 1) ^
                (std::hash<bool>()(layout.hasTangents) << 2) ^
                (std::hash<bool>()(layout.hasBinormals) << 3) ^
                (std::hash<bool>()(layout.hasUVs) << 4));
        }
    };
}
