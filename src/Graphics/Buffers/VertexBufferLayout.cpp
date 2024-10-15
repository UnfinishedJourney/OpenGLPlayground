#include "VertexBufferLayout.h"
#include "Utilities/Logger.h" 

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
    GLenum type = GL_FLOAT;
    unsigned char normalized = 0;

    m_Elements.push_back({ type, count, normalized, VertexBufferElement::GetSizeOfType(type) });
    m_Stride += count * VertexBufferElement::GetSizeOfType(type);

    Logger::GetLogger()->info("Added VertexBufferElement: type=GL_FLOAT, count={}, normalized={}, size={}. Stride is now {}.",
        count, static_cast<unsigned int>(normalized), VertexBufferElement::GetSizeOfType(type), m_Stride);
}

template<>
void VertexBufferLayout::Push<unsigned int>(unsigned int count)
{
    GLenum type = GL_UNSIGNED_INT;
    unsigned char normalized = 0;

    m_Elements.push_back({ type, count, normalized, VertexBufferElement::GetSizeOfType(type) });
    m_Stride += count * VertexBufferElement::GetSizeOfType(type);

    Logger::GetLogger()->info("Added VertexBufferElement: type=GL_UNSIGNED_INT, count={}, normalized={}, size={}. Stride is now {}.",
        count, static_cast<unsigned int>(normalized), VertexBufferElement::GetSizeOfType(type), m_Stride);
}

template<>
void VertexBufferLayout::Push<unsigned char>(unsigned int count)
{
    GLenum type = GL_UNSIGNED_BYTE;
    unsigned char normalized = 1;

    m_Elements.push_back({ type, count, normalized, VertexBufferElement::GetSizeOfType(type) });
    m_Stride += count * VertexBufferElement::GetSizeOfType(type);

    Logger::GetLogger()->info("Added VertexBufferElement: type=GL_UNSIGNED_BYTE, count={}, normalized={}, size={}. Stride is now {}.",
        count, static_cast<unsigned int>(normalized), VertexBufferElement::GetSizeOfType(type), m_Stride);
}

template<typename T>
void VertexBufferLayout::Push(unsigned int count, size_t size, bool b_normalized)
{
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

    unsigned char normalized = b_normalized ? 1 : 0;

    m_Elements.push_back({ type, count, normalized, size });
    m_Stride += count * VertexBufferElement::GetSizeOfType(type);

    Logger::GetLogger()->info("Added VertexBufferElement: type={}, count={}, normalized={}, size={}. Stride is now {}.",
        type, count, static_cast<unsigned int>(normalized), size, m_Stride);
}