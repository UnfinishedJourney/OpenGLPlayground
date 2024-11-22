#pragma once

#include <vector>
#include <glad/glad.h>
#include <type_traits>

struct VertexBufferElement {
    GLuint attributeIndex;
    GLuint count;
    GLenum type;
    GLboolean normalized;
    GLuint offset;
};

template <typename T>
concept SupportedVertexType = std::is_same_v<T, float> ||
std::is_same_v<T, GLuint> ||
std::is_same_v<T, GLubyte>;

class VertexBufferLayout {
public:
    VertexBufferLayout() = default;

    template<SupportedVertexType T>
    void Push(GLuint count, GLuint attributeIndex);

    [[nodiscard]] const std::vector<VertexBufferElement>& GetElements() const { return m_Elements; }
    [[nodiscard]] GLuint GetStride() const { return m_Stride; }

private:
    std::vector<VertexBufferElement> m_Elements;
    GLuint m_Stride = 0;
    GLuint m_Offset = 0;
};