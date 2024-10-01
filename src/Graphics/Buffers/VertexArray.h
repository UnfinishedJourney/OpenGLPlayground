#pragma once

#include "Graphics/Buffers/VertexBuffer.h"

class VertexBufferLayout;

class VertexArray
{
public:
	VertexArray();
	VertexArray(unsigned int rendererID);
	~VertexArray();
	
	void AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout) const;
	void AddInstancedBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout, unsigned int divisor) const;
	void Bind() const;
	void Unbind() const;

private:
	unsigned int m_RendererID;
};