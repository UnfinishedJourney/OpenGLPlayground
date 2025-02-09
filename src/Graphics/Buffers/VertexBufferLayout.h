#pragma once

#include <vector>
#include <glad/glad.h>
#include <type_traits>

namespace Graphics {

    /**
     * @brief Describes a single vertex attribute.
     */
    struct VertexBufferElement {
        GLuint attributeIndex; ///< Location of the attribute in the shader
        GLuint count;          ///< Number of components (e.g., 3 for vec3)
        GLenum type;           ///< Data type (GL_FLOAT, GL_UNSIGNED_INT, etc.)
        GLboolean normalized;  ///< Whether fixed-point data should be normalized
        GLuint offset;         ///< Byte offset within the vertex structure
    };

    /**
     * @brief Concept for types that can be used in a vertex buffer.
     */
    template <typename T>
    concept SupportedVertexType = std::is_same_v<T, float> ||
        std::is_same_v<T, GLuint> ||
        std::is_same_v<T, GLubyte>;

    /**
     * @brief Manages a collection of vertex attributes and calculates stride.
     */
    class VertexBufferLayout {
    public:
        VertexBufferLayout() = default;

        /**
         * @brief Adds an attribute to the layout.
         * @tparam T Must satisfy SupportedVertexType.
         * @param count Number of components.
         * @param attributeIndex Attribute location in the shader.
         */
        template<SupportedVertexType T>
        void Push(GLuint count, GLuint attributeIndex);

        [[nodiscard]] const std::vector<VertexBufferElement>& GetElements() const { return m_Elements; }
        [[nodiscard]] GLuint GetStride() const { return m_Stride; }

    private:
        std::vector<VertexBufferElement> m_Elements;
        GLuint m_Stride{ 0 };
        GLuint m_Offset{ 0 };
    };

    template<SupportedVertexType T>
    void VertexBufferLayout::Push(GLuint count, GLuint attributeIndex)
    {
        if constexpr (std::is_same_v<T, float>) {
            m_Elements.push_back({ attributeIndex, count, GL_FLOAT, GL_FALSE, m_Offset });
            m_Offset += count * sizeof(GLfloat);
            m_Stride += count * sizeof(GLfloat);
        }
        else if constexpr (std::is_same_v<T, GLuint>) {
            m_Elements.push_back({ attributeIndex, count, GL_UNSIGNED_INT, GL_FALSE, m_Offset });
            m_Offset += count * sizeof(GLuint);
            m_Stride += count * sizeof(GLuint);
        }
        else if constexpr (std::is_same_v<T, GLubyte>) {
            m_Elements.push_back({ attributeIndex, count, GL_UNSIGNED_BYTE, GL_TRUE, m_Offset });
            m_Offset += count * sizeof(GLubyte);
            m_Stride += count * sizeof(GLubyte);
        }
    }

} // namespace Graphics