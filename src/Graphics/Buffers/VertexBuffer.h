#pragma once

class VertexBuffer
{
public:
	VertexBuffer(const void* data, unsigned int size);
	VertexBuffer(unsigned int rendererID);
	~VertexBuffer();
	unsigned int GetRendererID() const
	{
		return m_RendererID;
	}

	void Bind() const;
	void Unbind() const;

private:
	unsigned int m_RendererID;
	size_t m_Offset = 0;
};