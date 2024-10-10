#pragma once

#include <vector>

class IndexBuffer
{
public:
	IndexBuffer(const std::vector<unsigned int>& data, unsigned int count);
	IndexBuffer(unsigned int rendererID, unsigned int count);
	~IndexBuffer();

	void Bind() const;
	void Unbind() const;
	unsigned int GetCount() const;

private:
	unsigned int m_RendererID;
	unsigned int m_Count;
};