#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexBufferLayout.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <algorithm>
#include <stdexcept>

namespace Graphics {

    VertexArray::VertexArray()
    {
        GLCall(glCreateVertexArrays(1, &m_RendererID));
        if (m_RendererID == 0) {
            Logger::GetLogger()->error("Failed to create Vertex Array Object.");
            throw std::runtime_error("Failed to create Vertex Array Object.");
        }
        Logger::GetLogger()->info("Created VertexArray (ID={}).", m_RendererID);
    }

    VertexArray::~VertexArray()
    {
        if (m_RendererID != 0) {
            GLCall(glDeleteVertexArrays(1, &m_RendererID));
            Logger::GetLogger()->info("Deleted VertexArray (ID={}).", m_RendererID);
        }
    }

    VertexArray::VertexArray(VertexArray&& other) noexcept
        : m_RendererID(other.m_RendererID),
        m_BindingIndices(std::move(other.m_BindingIndices))
    {
        other.m_RendererID = 0;
    }

    VertexArray& VertexArray::operator=(VertexArray&& other) noexcept
    {
        if (this != &other) {
            if (m_RendererID != 0) {
                GLCall(glDeleteVertexArrays(1, &m_RendererID));
            }
            m_RendererID = other.m_RendererID;
            m_BindingIndices = std::move(other.m_BindingIndices);
            other.m_RendererID = 0;
        }
        return *this;
    }

    void VertexArray::AddBuffer(const VertexBuffer& vertexBuffer,
        const VertexBufferLayout& layout,
        GLuint bindingIndex)
    {
        // Prevent using the same binding index more than once.
        if (std::find(m_BindingIndices.begin(), m_BindingIndices.end(), bindingIndex) != m_BindingIndices.end()) {
            Logger::GetLogger()->error("Binding index={} is already used in VertexArray (ID={}).", bindingIndex, m_RendererID);
            throw std::invalid_argument("Duplicate binding index in VertexArray::AddBuffer");
        }

        // Attach the buffer at the specified binding index using DSA.
        GLCall(glVertexArrayVertexBuffer(m_RendererID, bindingIndex, vertexBuffer.GetRendererID(), 0, layout.GetStride()));

        // Configure each vertex attribute defined in the layout.
        for (const auto& element : layout.GetElements()) {
            GLuint attribIndex = element.attributeIndex;
            GLCall(glEnableVertexArrayAttrib(m_RendererID, attribIndex));
            GLCall(glVertexArrayAttribFormat(m_RendererID, attribIndex, element.count, element.type, element.normalized, element.offset));
            GLCall(glVertexArrayAttribBinding(m_RendererID, attribIndex, bindingIndex));
        }

        m_BindingIndices.push_back(bindingIndex);
        Logger::GetLogger()->info("Added VertexBuffer (ID={}) to VertexArray (ID={}) at binding index={}.", vertexBuffer.GetRendererID(), m_RendererID, bindingIndex);
    }

    void VertexArray::SetIndexBuffer(const IndexBuffer& indexBuffer)
    {
        GLCall(glVertexArrayElementBuffer(m_RendererID, indexBuffer.GetRendererID()));
        Logger::GetLogger()->info("Set IndexBuffer (ID={}) to VertexArray (ID={}).", indexBuffer.GetRendererID(), m_RendererID);
    }

    void VertexArray::Bind() const
    {
        GLCall(glBindVertexArray(m_RendererID));
    }

    void VertexArray::Unbind() const
    {
        GLCall(glBindVertexArray(0));
    }

} // namespace Graphics