#pragma once

#include <vector>
#include <glad/glad.h>

struct VertexBufferElement {
    GLuint count;
    GLenum type;
    GLboolean normalized;
    GLuint offset;

    static GLuint GetSizeOfType(GLenum type) {
        switch (type) {
        case GL_FLOAT: return sizeof(GLfloat);
        case GL_UNSIGNED_INT: return sizeof(GLuint);
        case GL_UNSIGNED_BYTE: return sizeof(GLubyte);
        default: return 0;
        }
    }
};

class VertexBufferLayout {
public:
    VertexBufferLayout() = default;

    template<typename T>
    void Push(GLuint count);

    [[nodiscard]] const std::vector<VertexBufferElement>& GetElements() const { return m_Elements; }
    [[nodiscard]] GLuint GetStride() const { return m_Stride; }

private:
    std::vector<VertexBufferElement> m_Elements;
    GLuint m_Stride = 0;
    GLuint m_Offset = 0;
};