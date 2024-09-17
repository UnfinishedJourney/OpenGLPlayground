#include <glad/glad.h>

#include "IndexBuffer.h"
#include "Utility.h"

IndexBuffer::IndexBuffer(const unsigned int* data, unsigned int count)
	: m_Count(count), m_IsBound(false)
{
	ASSERT(sizeof(unsigned int) == sizeof(GLuint));

	GLCall(glGenBuffers(1, &m_RendererID));
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID));
	GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW));
}

IndexBuffer::IndexBuffer(unsigned int rendererID, unsigned int count)
	: m_RendererID(rendererID), m_Count(count), m_IsBound(false)
{
}

IndexBuffer::~IndexBuffer() {
	GLCall(glDeleteBuffers(1, &m_RendererID));
}

void IndexBuffer::Bind() const {
	if (m_IsBound)
		return;
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID));
	m_IsBound = true;
}

void IndexBuffer::Unbind() const {
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
	m_IsBound = false;
}

unsigned int IndexBuffer::GetCount() const
{
	return m_Count;
}
