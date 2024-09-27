#include "VertexArray.h"
#include "Utility.h"
#include "VertexBufferLayout.h"

VertexArray::VertexArray() 
{
    GLCall(glGenVertexArrays(1, &m_RendererID));
}

VertexArray::VertexArray(unsigned int rendererID)
    : m_RendererID(rendererID)
{
}


VertexArray::~VertexArray()
{
    GLCall(glDeleteVertexArrays(1, &m_RendererID));
}

void VertexArray::AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout) const
{
    Bind();
    vb.Bind();
    const auto& elements = layout.GetElements();
    unsigned int offset = 0;
    for (unsigned int i = 0; i < elements.size(); i++) {
        const auto& element = elements[i];
        GLCall(glEnableVertexAttribArray(i));
        GLCall(glVertexAttribPointer(i, element.count, element.type,
            element.normalized, layout.GetStride(), (const void*)offset));
        offset += element.count * VertexBufferElement::GetSizeOfType(element.type);
    }

    Unbind();
}

void VertexArray::AddInstancedBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout, unsigned int divisor) const
{
    Bind();
    vb.Bind();
    const auto& elements = layout.GetElements();
    unsigned int offset = 0;

    for (unsigned int i = 0; i < elements.size(); i++) {
        const auto& element = elements[i];
        GLCall(glEnableVertexAttribArray(i));
        GLCall(glVertexAttribPointer(i, element.count, element.type,
            element.normalized, layout.GetStride(), (const void*)offset));
        GLCall(glVertexAttribDivisor(i, divisor)); 
        offset += element.count * VertexBufferElement::GetSizeOfType(element.type);
    }

    Unbind();
}

void VertexArray::Bind() const
{
    GLCall(glBindVertexArray(m_RendererID));
}


void VertexArray::Unbind() const
{
    GLCall(glBindVertexArray(0));
}
