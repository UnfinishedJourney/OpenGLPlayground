#include "VertexArray.h"
#include "Utility.h"
#include "VertexBufferLayout.h"

VertexArray::VertexArray() 
    : m_IsBound(false)
{
    GLCall(glGenVertexArrays(1, &m_RendererID));
}

VertexArray::VertexArray(unsigned int rendererID)
    : m_RendererID(rendererID), m_IsBound(false)
{
}


VertexArray::~VertexArray()
{
    GLCall(glDeleteVertexArrays(1, &m_RendererID));
}

void VertexArray::AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout)
{
    Bind();
    vb.Bind();
    const auto& elements = layout.GetElements();
    unsigned int offset = 0;
    if (layout.IsSubBuffered())
    {
        for (unsigned int i = 0; i < elements.size(); i++) {
            const auto& element = elements[i];
            GLCall(glEnableVertexAttribArray(i));
            GLCall(glVertexAttribPointer(i, element.count, element.type,
                element.normalized, element.count * VertexBufferElement::GetSizeOfType(element.type), (const void*)offset));
            offset += element.size;
        }
    }
    else
    {
        for (unsigned int i = 0; i < elements.size(); i++) {
            const auto& element = elements[i];
            GLCall(glEnableVertexAttribArray(i));
            GLCall(glVertexAttribPointer(i, element.count, element.type,
                element.normalized, layout.GetStride(), (const void*)offset));
            offset += element.count * VertexBufferElement::GetSizeOfType(element.type);
        }
    }

    Unbind();
}

void VertexArray::Bind() const
{
    if (m_IsBound)
        return;
    GLCall(glBindVertexArray(m_RendererID));
    m_IsBound = true;
}


void VertexArray::Unbind() const
{
    GLCall(glBindVertexArray(0));
    m_IsBound = false;
}
