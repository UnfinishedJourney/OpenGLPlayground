#pragma once

#include "glm/glm.hpp"

//struct Light
//{
//public:
//	glm::vec4 position;
//	glm::vec3 color;
//};

struct LightData {
    glm::vec4 position; // Use .w as padding or another parameter
    glm::vec4 color;    // Use .w as intensity
};