#pragma once

#include "Utilities/Utility.h"
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glad/glad.h>

struct Mesh {
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> tangents;
    std::vector<glm::vec3> bitangents;
    std::unordered_map<TextureType, std::vector<glm::vec2>> uvs;
    std::vector<GLuint> indices;
};