#pragma once
#include "Graphics/Buffers/VertexBuffer.h"
#include "Graphics/Buffers/VertexBufferLayout.h"
#include "Graphics/Buffers/IndexBuffer.h"
#include <memory>

class SkyBox
{
public:
    SkyBox(GLuint texID, float size = 50.0f);
	void Draw(const std::unique_ptr<Shader>& shader) const;
private:
	std::unique_ptr<VertexArray> m_VAO;
	std::unique_ptr<VertexBuffer> m_VB;
	std::unique_ptr<IndexBuffer> m_IB;
	float m_Size = 50.0;
	GLuint m_TexID;
};