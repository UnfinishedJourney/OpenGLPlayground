#include <GL/glew.h>

#include "VertexBuffer.h"
#include "Utility.h"

VertexBuffer::VertexBuffer(const void* data, unsigned int size) 
{
	GLCall(glGenBuffers(1, &m_RendererID));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
	GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
}

VertexBuffer::VertexBuffer(unsigned int rendererID) 
	: m_RendererID(rendererID)
{
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
}

VertexBuffer::~VertexBuffer() 
{
	GLCall(glDeleteBuffers(1, &m_RendererID));
}

void VertexBuffer::Bind() const
{
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
}

void VertexBuffer::Unbind() const
{
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void VertexBuffer::AddSubData(const void* data, unsigned int size)
{
	GLCall(glBufferSubData(GL_ARRAY_BUFFER, m_Offset, size, data));
	m_Offset += size;
}
