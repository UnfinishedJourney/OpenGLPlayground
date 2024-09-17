#pragma once

class IndexBuffer
{
public:
	IndexBuffer(const unsigned int* data, unsigned int count);
	IndexBuffer(unsigned int rendererID, unsigned int count);
	~IndexBuffer();

	void Bind() const;
	void Unbind() const;
	unsigned int GetCount() const;
	bool GetStatus() const
	{
		return m_IsBound;
	}

private:
	unsigned int m_RendererID;
	unsigned int m_Count;
	mutable bool m_IsBound;
};