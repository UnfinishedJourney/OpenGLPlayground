#pragma once

#include <variant>
#include <glm.hpp>

// Define UniformValue as a variant of possible uniform types
using UniformValue = std::variant<int, float, glm::vec2, glm::vec3, glm::vec4, glm::mat3, glm::mat4>;