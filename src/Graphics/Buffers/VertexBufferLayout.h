#pragma once

#include <vector>
#include <glad/glad.h>

struct VertexBufferElement
{
	unsigned int type;
	unsigned int count;
	unsigned char normalized;
	size_t size = 1;

	static unsigned int GetSizeOfType(unsigned int type)
	{
		switch (type)
		{
		case GL_FLOAT:			return 4;
		case GL_UNSIGNED_INT:	return 4;
		case GL_UNSIGNED_BYTE:	return 1;
		}
		ASSERT(false);
		return 0;
	}
};

class VertexBufferLayout
{
public:
	VertexBufferLayout()
		: m_Stride(0)
	{}


	template<typename T>
	void Push(unsigned int count)
	{
		static_assert(false);
	}

	template<>
	void Push<float>(unsigned int count)
	{
		m_Elements.push_back({ GL_FLOAT, count, 0 });
		m_Stride += count * VertexBufferElement::GetSizeOfType(GL_FLOAT);
	}

	template<>
	void Push<unsigned int>(unsigned int count)
	{
		m_Elements.push_back({ GL_UNSIGNED_INT, count, 0 });
		m_Stride += count * VertexBufferElement::GetSizeOfType(GL_UNSIGNED_INT);
	}

	template<>
	void Push<unsigned char>(unsigned int count)
	{
		m_Elements.push_back({ GL_UNSIGNED_BYTE, count, 1 });
		m_Stride += count * VertexBufferElement::GetSizeOfType(GL_UNSIGNED_BYTE);
	}

	template<typename T>
	void Push(unsigned int count, size_t size, bool b_normalized = false)
	{
		m_Elements.push_back({ GL_FLOAT, count, b_normalized, size });
		m_Stride += count * VertexBufferElement::GetSizeOfType(GL_FLOAT);
	}

	inline std::vector<VertexBufferElement> GetElements() const
	{
		return m_Elements;
	}

	inline void SetStride(size_t stride)
	{
		m_Stride = stride;
	}

	inline unsigned int GetStride() const
	{
		return m_Stride;
	}
private:
	std::vector<VertexBufferElement> m_Elements;
	unsigned int m_Stride;
};