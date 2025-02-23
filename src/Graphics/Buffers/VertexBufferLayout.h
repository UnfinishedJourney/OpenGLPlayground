#pragma once

#include <vector>
#include <glad/glad.h>
#include <type_traits>

namespace graphics {

    /**
     * @brief Describes a single vertex attribute.
     */
    struct VertexBufferElement {
        GLuint attributeIndex_; ///< Location of the attribute in the shader.
        GLuint count_;           ///< Number of components (e.g., 3 for vec3).
        GLenum type_;            ///< Data type (GL_FLOAT, GL_UNSIGNED_INT, etc.).
        GLboolean normalized_;   ///< Whether fixed-point data should be normalized.
        GLuint offset_;          ///< Byte offset within the vertex structure.
    };

    /**
     * @brief Concept for types that can be used in a vertex buffer.
     */
    template <typename T>
    concept SupportedVertexType = std::is_same_v<T, float> ||
        std::is_same_v<T, GLuint> ||
        std::is_same_v<T, GLubyte>;

    /**
     * @brief Manages a collection of vertex attributes and calculates the stride.
     */
    class VertexBufferLayout {
    public:
        VertexBufferLayout() = default;

        /**
         * @brief Adds an attribute to the layout.
         * @tparam T Must satisfy SupportedVertexType.
         * @param count Number of components.
         * @param attribute_index Attribute location in the shader.
         */
        template<SupportedVertexType T>
        void Push(GLuint count, GLuint attributeIndex);

        [[nodiscard]] const std::vector<VertexBufferElement>& GetElements() const { return elements_; }
        [[nodiscard]] GLuint GetStride() const { return stride_; }

    private:
        std::vector<VertexBufferElement> elements_;
        GLuint stride_{ 0 };
        GLuint offset_{ 0 };
    };

    template<SupportedVertexType T>
    void VertexBufferLayout::Push(GLuint count, GLuint attributeIndex)
    {
        if constexpr (std::is_same_v<T, float>) {
            elements_.push_back({ attributeIndex, count, GL_FLOAT, GL_FALSE, offset_ });
            offset_ += count * sizeof(GLfloat);
            stride_ += count * sizeof(GLfloat);
        }
        else if constexpr (std::is_same_v<T, GLuint>) {
            elements_.push_back({ attributeIndex, count, GL_UNSIGNED_INT, GL_FALSE, offset_ });
            offset_ += count * sizeof(GLuint);
            stride_ += count * sizeof(GLuint);
        }
        else if constexpr (std::is_same_v<T, GLubyte>) {
            elements_.push_back({ attributeIndex, count, GL_UNSIGNED_BYTE, GL_TRUE, offset_ });
            offset_ += count * sizeof(GLubyte);
            stride_ += count * sizeof(GLubyte);
        }
    }

} // namespace graphics