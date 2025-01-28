#pragma once

#include <vector>
#include <glad/glad.h>
#include <type_traits>

/**
 * @brief Describes a single vertex attribute in the layout (e.g., position, normal).
 */
struct VertexBufferElement {
    GLuint attributeIndex;
    GLuint count;       ///< Number of components (e.g., 3 for a vec3)
    GLenum type;        ///< GL_FLOAT, GL_UNSIGNED_INT, etc.
    GLboolean normalized;
    GLuint offset;      ///< Byte offset within the vertex
};

/**
 * @brief Concept restricting types to those suitable for use in a vertex buffer.
 */
template <typename T>
concept SupportedVertexType = std::is_same_v<T, float> ||
std::is_same_v<T, GLuint> ||
std::is_same_v<T, GLubyte>;

/**
 * @brief Stores a list of VertexBufferElements and calculates stride/offset.
 */
class VertexBufferLayout {
public:
    VertexBufferLayout() = default;

    /**
     * @brief Add an attribute (e.g., float3) to the layout.
     * @tparam T        Must be one of the SupportedVertexType.
     * @param count     Number of components (e.g., 3 for vec3).
     * @param attributeIndex The attribute location/index to bind.
     */
    template<SupportedVertexType T>
    void Push(GLuint count, GLuint attributeIndex);

    [[nodiscard]] const std::vector<VertexBufferElement>& GetElements() const { return m_Elements; }
    [[nodiscard]] GLuint GetStride() const { return m_Stride; }

private:
    std::vector<VertexBufferElement> m_Elements;
    GLuint                           m_Stride = 0;
    GLuint                           m_Offset = 0;
};