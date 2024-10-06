#pragma once

#include "Utilities/Utility.h"
#include "Graphics/Buffers/VertexArray.h"
#include "Graphics/Buffers/VertexBuffer.h"
#include "Graphics/Buffers/VertexBufferLayout.h"
#include "Graphics/Buffers/IndexBuffer.h"
#include "glm.hpp"

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>

struct Mesh
{
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> bitangents;
    std::vector<glm::vec3> tangents;
    std::unordered_map<TextureType, std::vector<glm::vec2>> uvs;
    std::vector<unsigned int> indices;
};

struct MeshLayout
{
    bool hasPositions;
    bool hasNormals;
    bool hasBitangents;
    bool hasTangents;
    std::unordered_set<TextureType> textureTypes;

    inline bool operator==(const MeshLayout& other) const {
        return hasPositions == other.hasPositions &&
            hasNormals == other.hasNormals &&
            hasTangents == other.hasTangents &&
            hasBitangents == other.hasBitangents &&
            textureTypes == other.textureTypes;
    }

};

class MeshBuffer {
public:

    MeshBuffer(std::shared_ptr<VertexArray> vao, std::shared_ptr<VertexBuffer> vb, std::shared_ptr<IndexBuffer> ib, size_t nVerts)
        : m_VAO(std::move(vao)), m_VB(std::move(vb)), m_IB(std::move(ib)), m_NVerts(nVerts)
    {}

    MeshBuffer(std::shared_ptr<Mesh> mesh, const MeshLayout& mLayout);

    void Bind() const;
    void Unbind() const;
    size_t GetNVerts() const
    {
        return m_NVerts;
    }
    std::shared_ptr<VertexArray> GetVAO() const 
    { 
        return m_VAO; 
    }

private:
    std::shared_ptr<VertexArray> m_VAO;
    std::shared_ptr<VertexBuffer> m_VB;
    std::shared_ptr<IndexBuffer> m_IB;
    size_t m_NVerts;
};


namespace std {
    template <>
    struct hash<MeshLayout> {
        std::size_t operator()(const MeshLayout& mLayout) const {
            std::size_t seed = 0;
            seed ^= std::hash<bool>()(mLayout.hasPositions) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= std::hash<bool>()(mLayout.hasNormals) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= std::hash<bool>()(mLayout.hasTangents) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= std::hash<bool>()(mLayout.hasBitangents) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

            for (const auto& texType : mLayout.textureTypes) {
                seed ^= std::hash<int>()(static_cast<int>(texType)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }

            return seed;
        }
    };
}
