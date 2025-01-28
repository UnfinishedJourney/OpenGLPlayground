#include "VertexBufferLayout.h"

template<SupportedVertexType T>
void VertexBufferLayout::Push(GLuint count, GLuint attributeIndex)
{
    if constexpr (std::is_same_v<T, float>) {
        m_Elements.push_back(
            { attributeIndex, count, GL_FLOAT, GL_FALSE, m_Offset }
        );
        m_Offset += count * sizeof(GLfloat);
        m_Stride += count * sizeof(GLfloat);
    }
    else if constexpr (std::is_same_v<T, GLuint>) {
        m_Elements.push_back(
            { attributeIndex, count, GL_UNSIGNED_INT, GL_FALSE, m_Offset }
        );
        m_Offset += count * sizeof(GLuint);
        m_Stride += count * sizeof(GLuint);
    }
    else if constexpr (std::is_same_v<T, GLubyte>) {
        m_Elements.push_back(
            { attributeIndex, count, GL_UNSIGNED_BYTE, GL_TRUE, m_Offset }
        );
        m_Offset += count * sizeof(GLubyte);
        m_Stride += count * sizeof(GLubyte);
    }
}

// Explicitly instantiate for supported types
template void VertexBufferLayout::Push<float>(GLuint, GLuint);
template void VertexBufferLayout::Push<GLuint>(GLuint, GLuint);
template void VertexBufferLayout::Push<GLubyte>(GLuint, GLuint);
