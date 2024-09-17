#include <glad/glad.h>

#include "VertexBuffer.h"
#include "Utility.h"

VertexBuffer::VertexBuffer(const void* data, unsigned int size) 
	: m_IsBound(false)
{
	GLCall(glGenBuffers(1, &m_RendererID));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
	GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
}

VertexBuffer::VertexBuffer(unsigned int rendererID) 
	: m_RendererID(rendererID), m_IsBound(false)
{
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
}

VertexBuffer::~VertexBuffer() 
{
	GLCall(glDeleteBuffers(1, &m_RendererID));
}

void VertexBuffer::Bind() const
{
	if (m_IsBound)
		return;
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
	m_IsBound = true;
}

void VertexBuffer::Unbind() const
{
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
	m_IsBound = false;
}

void VertexBuffer::AddSubData(const void* data, unsigned int size)
{
	GLCall(glBufferSubData(GL_ARRAY_BUFFER, m_Offset, size, data));
	m_Offset += size;
}
