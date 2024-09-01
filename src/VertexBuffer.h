#pragma once

class VertexBuffer
{
private:
	unsigned int m_RendererID;
	size_t m_Offset = 0;
public:
	VertexBuffer(const void* data, unsigned int size);
	VertexBuffer(unsigned int rendererID);
	~VertexBuffer();

	void Bind() const;
	void Unbind() const;

	void AddSubData(const void* data, unsigned int size);

};