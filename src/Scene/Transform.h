#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include <algorithm> 
#include <functional>

class Transform {
public:
    Transform()
        : m_ModelMatrix(1.0f), m_NormalMatrix(1.0)
    {}

    glm::mat4 GetModelMatrix() const {
        return m_ModelMatrix;
    }

    glm::mat3 GetNormalMatrix() const {
        return m_NormalMatrix;
    }

    void SetModelMatrix(const glm::mat4& matrix)
    {
        m_ModelMatrix = matrix;
        m_NormalMatrix = glm::transpose(glm::inverse(glm::mat3(matrix)));
    }

    bool operator==(const Transform& other) const {
        return m_ModelMatrix == other.m_ModelMatrix;
    }

    // Optionally define inequality operator.
    bool operator!=(const Transform& other) const {
        return !(*this == other);
    }

private:
    glm::mat3 m_NormalMatrix;
    glm::mat4 m_ModelMatrix;
};

namespace std {

    template<>
    struct hash<Transform> {
        size_t operator()(const Transform& transform) const {
            // Retrieve the model matrix.
            glm::mat4 model = transform.GetModelMatrix();
            size_t seed = 0;
            // Combine the hash for each float.
            for (size_t i = 0; i < 4; ++i) {
                for (size_t j = 0; j < 4; ++j)
                    seed ^= std::hash<float>{}(model[i][j]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            return seed;
        }
    };
}