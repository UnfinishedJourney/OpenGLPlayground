#pragma once

#include "Utilities/Utility.h"
#include <glad/glad.h>

#include <vector>

struct VertexBufferElement
{
    unsigned int type;
    unsigned int count;
    unsigned char normalized;
    size_t size = 1;

    static unsigned int GetSizeOfType(unsigned int type)
    {
        switch (type)
        {
        case GL_UNSIGNED_BYTE: return 1;
        default:               return 4;
        }
        ASSERT(false);
        return 0;
    }
};

class VertexBufferLayout
{
public:
    VertexBufferLayout()
        : m_Stride(0)
    {}

    template<typename T>
    void Push(unsigned int count);

    template<typename T>
    void Push(unsigned int count, size_t size, bool b_normalized = false);

    inline const std::vector<VertexBufferElement>& GetElements() const
    {
        return m_Elements;
    }

    inline void SetStride(size_t stride)
    {
        m_Stride = stride;
    }

    inline unsigned int GetStride() const
    {
        return m_Stride;
    }

private:
    std::vector<VertexBufferElement> m_Elements;
    unsigned int m_Stride;
};
