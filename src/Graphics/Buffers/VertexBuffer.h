#pragma once

class VertexBuffer
{
public:
	VertexBuffer(const void* data, unsigned int size);
	VertexBuffer(unsigned int rendererID);
	~VertexBuffer();
	void Bind() const;
	void Unbind() const;

private:
	unsigned int m_RendererID;
	size_t m_Offset = 0;
};