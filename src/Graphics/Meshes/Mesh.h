#pragma once

#include <variant>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include <cstdint>
#include <type_traits>
#include <stdexcept>
#include <cfloat>
#include "Utilities/Utility.h"

enum class TextureType {
    Albedo,
    Normal,
    MetalRoughness,
    AO,         // Ambient Occlusion
    Emissive
};

struct MeshLOD {
    uint32_t indexOffset; // Offset into the combined index buffer
    uint32_t indexCount;  // Number of indices for this LOD
};

struct Mesh {
    using PositionsType = std::variant<std::vector<glm::vec2>, std::vector<glm::vec3>>;

    PositionsType positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> tangents;
    std::vector<glm::vec3> bitangents;
    std::unordered_map<TextureType, std::vector<glm::vec2>> uvs;
    std::vector<uint32_t> indices;
    std::vector<MeshLOD> lods;

    glm::vec3 localCenter = glm::vec3(0.0f);
    float boundingSphereRadius = 1.0f;

    glm::vec3 minBounds = glm::vec3(FLT_MAX);
    glm::vec3 maxBounds = glm::vec3(-FLT_MAX);

    size_t GetVertexCount() const {
        return std::visit([](auto&& arg) -> size_t {
            return arg.size();
            }, positions);
    }

    size_t GetLODCount() const { return lods.size(); }

    bool HasNormals() const { return !normals.empty(); }
    bool HasTangents() const { return !tangents.empty(); }
    bool HasBitangents() const { return !bitangents.empty(); }

    bool HasTextureCoords(TextureType type) const {
        auto it = uvs.find(type);
        return it != uvs.end() && !it->second.empty();
    }

    Mesh() = default;

    Mesh(Mesh&& other) noexcept
        : positions(std::move(other.positions)),
        normals(std::move(other.normals)),
        tangents(std::move(other.tangents)),
        bitangents(std::move(other.bitangents)),
        uvs(std::move(other.uvs)),
        indices(std::move(other.indices)),
        lods(std::move(other.lods)),
        localCenter(other.localCenter),
        boundingSphereRadius(other.boundingSphereRadius),
        minBounds(other.minBounds),
        maxBounds(other.maxBounds) {}

    Mesh& operator=(Mesh&& other) noexcept {
        if (this != &other) {
            positions = std::move(other.positions);
            normals = std::move(other.normals);
            tangents = std::move(other.tangents);
            bitangents = std::move(other.bitangents);
            uvs = std::move(other.uvs);
            indices = std::move(other.indices);
            lods = std::move(other.lods);
            localCenter = other.localCenter;
            boundingSphereRadius = other.boundingSphereRadius;
            minBounds = other.minBounds;
            maxBounds = other.maxBounds;
        }
        return *this;
    }

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
};