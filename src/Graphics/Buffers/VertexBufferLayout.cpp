#include "VertexBufferLayout.h"

#include <vector>
#include <glad/glad.h>

template<typename T>
void VertexBufferLayout::Push(unsigned int count)
{
    static_assert(sizeof(T) == 0, "Unsupported type for VertexBufferLayout::Push");
}

template<>
void VertexBufferLayout::Push<float>(unsigned int count)
{
    m_Elements.push_back({ GL_FLOAT, count, 0 });
    m_Stride += count * VertexBufferElement::GetSizeOfType(GL_FLOAT);
}

template<>
void VertexBufferLayout::Push<unsigned int>(unsigned int count)
{
    m_Elements.push_back({ GL_UNSIGNED_INT, count, 0 });
    m_Stride += count * VertexBufferElement::GetSizeOfType(GL_UNSIGNED_INT);
}

template<>
void VertexBufferLayout::Push<unsigned char>(unsigned int count)
{
    m_Elements.push_back({ GL_UNSIGNED_BYTE, count, 1 });
    m_Stride += count * VertexBufferElement::GetSizeOfType(GL_UNSIGNED_BYTE);
}

template<typename T>
void VertexBufferLayout::Push(unsigned int count, size_t size, bool b_normalized)
{
    // Example implementation, adjust as needed
    unsigned int type;
    if constexpr (std::is_same_v<T, float>) {
        type = GL_FLOAT;
    }
    else if constexpr (std::is_same_v<T, unsigned int>) {
        type = GL_UNSIGNED_INT;
    }
    else if constexpr (std::is_same_v<T, unsigned char>) {
        type = GL_UNSIGNED_BYTE;
    }
    else {
        static_assert(sizeof(T) == 0, "Unsupported type for VertexBufferLayout::Push with size");
    }

    m_Elements.push_back({ type, count, static_cast<unsigned char>(b_normalized), size });
    m_Stride += count * VertexBufferElement::GetSizeOfType(type);
}