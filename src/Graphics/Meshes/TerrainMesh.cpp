#include "TerrainMesh.h"

TerrainMesh::TerrainMesh() {
    positions = std::vector<glm::vec3>{
        glm::vec3(-1.0f, 0.0f, -1.0f),
        glm::vec3(1.0f, 0.0f, -1.0f),
        glm::vec3(1.0f, 0.0f,  1.0f),
        glm::vec3(-1.0f, 0.0f,  1.0f)
    };

    uvs[TextureType::Albedo] = {
        glm::vec2(0.0f, 0.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(1.0f, 1.0f),
        glm::vec2(0.0f, 1.0f)
    };

    indices = { 0, 1, 2, 2, 3, 0 };
}