#pragma once

#include "VertexBuffer.h"

class VertexBufferLayout;

class VertexArray
{
public:
	VertexArray();
	VertexArray(unsigned int rendererID);
	~VertexArray();
	
	void AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);
	void Bind() const;
	void Unbind() const;
	bool GetStatus() const
	{
		return m_IsBound;
	}

private:
	unsigned int m_RendererID;
	mutable bool m_IsBound;
};