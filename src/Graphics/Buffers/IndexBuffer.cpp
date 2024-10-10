#include <glad/glad.h>
#include "Graphics/Buffers/IndexBuffer.h"
#include "Utilities/Utility.h"
#include "Utilities/Logger.h" 

IndexBuffer::IndexBuffer(const std::vector<unsigned int>& data, unsigned int count)
    : m_Count(count)
{
    Logger::GetLogger()->info("Creating IndexBuffer with {} indices.", count);

    ASSERT(sizeof(unsigned int) == sizeof(GLuint));
    Logger::GetLogger()->debug("Verified size of unsigned int matches GLuint.");

    GLCall(glGenBuffers(1, &m_RendererID));
    Logger::GetLogger()->info("Generated OpenGL Index Buffer with ID {}.", m_RendererID);

    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID));
    Logger::GetLogger()->debug("Bound Index Buffer ID {}.", m_RendererID);

    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data.data(), GL_STATIC_DRAW));
    Logger::GetLogger()->info("Uploaded {} bytes of index data to Buffer ID {}.", count * sizeof(unsigned int), m_RendererID);
}

IndexBuffer::IndexBuffer(unsigned int rendererID, unsigned int count)
    : m_RendererID(rendererID), m_Count(count)
{
    Logger::GetLogger()->info("Creating IndexBuffer with existing Renderer ID {} and {} indices.", rendererID, count);
}

IndexBuffer::~IndexBuffer()
{
    Logger::GetLogger()->info("Deleting IndexBuffer with Renderer ID {}.", m_RendererID);
    GLCall(glDeleteBuffers(1, &m_RendererID));
}

void IndexBuffer::Bind() const
{
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID));
    Logger::GetLogger()->debug("Bound Index Buffer ID {}.", m_RendererID);
}

void IndexBuffer::Unbind() const
{
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    Logger::GetLogger()->debug("Unbound Index Buffer.");
}

unsigned int IndexBuffer::GetCount() const
{
    Logger::GetLogger()->debug("Retrieving count: {}.", m_Count);
    return m_Count;
}