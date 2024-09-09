#pragma once
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"
#include <memory>

class SkyBox
{
public:
    SkyBox(float size = 50.0f);
	std::unique_ptr<VertexArray> m_VAO;
	std::unique_ptr<VertexBuffer> m_VB;
	std::unique_ptr<IndexBuffer> m_IB;
	void Draw(const std::unique_ptr<Shader>& shader, GLuint m_TexID) const;
};