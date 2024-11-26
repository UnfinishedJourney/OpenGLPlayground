#pragma once

#include <vector>
#include <unordered_map>
#include <variant>
#include <glm/glm.hpp>
#include <cstdint>
#include <type_traits>
#include <stdexcept>
#include "Utilities/Utility.h"

enum class TextureType {
    Albedo,
    Specular,
    Normal,
    Occlusion,
    RoughnessMetallic,
    Emissive,
};

namespace std {
    template <>
    struct hash<TextureType> {
        std::size_t operator()(const TextureType& type) const {
            return std::hash<int>()(static_cast<int>(type));
        }
    };
}

struct Mesh {
    using PositionsType = std::variant<std::vector<glm::vec2>, std::vector<glm::vec3>>;

    PositionsType positions; // Positions can be either vec2 or vec3
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> tangents;
    std::vector<glm::vec3> bitangents;
    std::unordered_map<TextureType, std::vector<glm::vec2>> uvs;
    std::vector<uint32_t> indices;

    // Utility methods
    size_t GetVertexCount() const {
        return std::visit([](auto&& arg) -> size_t { return arg.size(); }, positions);
    }

    size_t GetIndexCount() const { return indices.size(); }

    bool HasNormals() const { return !normals.empty(); }
    bool HasTangents() const { return !tangents.empty(); }
    bool HasBitangents() const { return !bitangents.empty(); }
    bool HasTextureCoords(TextureType type) const {
        auto it = uvs.find(type);
        return it != uvs.end() && !it->second.empty();
    }

    // Constructors
    Mesh() = default;

    // Move semantics
    Mesh(Mesh&& other) noexcept
        : positions(std::move(other.positions)),
        normals(std::move(other.normals)),
        tangents(std::move(other.tangents)),
        bitangents(std::move(other.bitangents)),
        uvs(std::move(other.uvs)),
        indices(std::move(other.indices)) {}

    Mesh& operator=(Mesh&& other) noexcept {
        if (this != &other) {
            positions = std::move(other.positions);
            normals = std::move(other.normals);
            tangents = std::move(other.tangents);
            bitangents = std::move(other.bitangents);
            uvs = std::move(other.uvs);
            indices = std::move(other.indices);
        }
        return *this;
    }

    // Delete copy constructor and copy assignment operator
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
};