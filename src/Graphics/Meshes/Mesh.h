#pragma once
#include <variant>
#include <vector>
#include <glm/glm.hpp>
#include <cfloat>
#include <unordered_map>
#include "Utilities/Utility.h" // if needed
// TextureType enum assumed defined
enum class TextureType {
    Albedo,
    Normal,
    MetalRoughness,
    AO,
    Emissive
};

struct MeshLOD {
    uint32_t indexOffset;
    uint32_t indexCount;
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

    size_t GetLODCount() const { return lods.size(); }
};