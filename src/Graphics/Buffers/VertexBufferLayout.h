#pragma once

#include <vector>
#include <glad/glad.h>

struct VertexBufferElement {
    GLuint attributeIndex; 
    GLuint count;
    GLenum type;
    GLboolean normalized;
    GLuint offset;
};

class VertexBufferLayout {
public:
    VertexBufferLayout() = default;

    template<typename T>
    void Push(GLuint count, GLuint attributeIndex);

    [[nodiscard]] const std::vector<VertexBufferElement>& GetElements() const { return m_Elements; }
    [[nodiscard]] GLuint GetStride() const { return m_Stride; }

private:
    std::vector<VertexBufferElement> m_Elements;
    GLuint m_Stride = 0;
    GLuint m_Offset = 0;
};