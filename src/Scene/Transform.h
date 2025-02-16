#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

class Transform {
public:
    Transform()
        : modelMatrix_(1.0f), normalMatrix_(1.0f)
    {}

    glm::mat4 GetModelMatrix() const { return modelMatrix_; }
    glm::mat3 GetNormalMatrix() const { return normalMatrix_; }

    void SetModelMatrix(const glm::mat4& matrix) {
        modelMatrix_ = matrix;
        normalMatrix_ = glm::transpose(glm::inverse(glm::mat3(matrix)));
    }

    bool operator==(const Transform& other) const {
        return modelMatrix_ == other.modelMatrix_;
    }

    bool operator!=(const Transform& other) const {
        return !(*this == other);
    }

private:
    glm::mat3 normalMatrix_;
    glm::mat4 modelMatrix_;
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