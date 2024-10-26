#include "VertexBufferLayout.h"
#include "Utilities/Logger.h" 

#include <vector>
#include <glad/glad.h>

template<typename T>
void VertexBufferLayout::Push(GLuint count) {
    static_assert(false, "Unsupported type for VertexBufferLayout::Push");
}

template<>
void VertexBufferLayout::Push<float>(GLuint count) {
    m_Elements.push_back({ count, GL_FLOAT, GL_FALSE, m_Offset });
    m_Offset += count * sizeof(GLfloat);
    m_Stride += count * sizeof(GLfloat);
}

template<>
void VertexBufferLayout::Push<GLuint>(GLuint count) {
    m_Elements.push_back({ count, GL_UNSIGNED_INT, GL_FALSE, m_Offset });
    m_Offset += count * sizeof(GLuint);
    m_Stride += count * sizeof(GLuint);
}

template<>
void VertexBufferLayout::Push<GLubyte>(GLuint count) {
    m_Elements.push_back({ count, GL_UNSIGNED_BYTE, GL_TRUE, m_Offset });
    m_Offset += count * sizeof(GLubyte);
    m_Stride += count * sizeof(GLubyte);
}