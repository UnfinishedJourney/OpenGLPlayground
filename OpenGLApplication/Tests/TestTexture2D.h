#pragma once

#include "Test.h"
#include "Shader.h"
#include "Texture.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"

#include <memory>

namespace test {
	class TestTexture2D : public Test
	{
	public:
		TestTexture2D();
		~TestTexture2D();

		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnImGuiRender() override;
	private:
		std::unique_ptr<VertexArray> m_VAO;
		std::unique_ptr<VertexBuffer> m_VB;
		std::unique_ptr<IndexBuffer> m_IB;
		std::unique_ptr<Shader> m_Shader;
		std::unique_ptr<Texture> m_Texture;

		glm::mat4 m_Proj;
		glm::mat4 m_View;
		glm::vec3 m_TranslationA, m_TranslationB;
	};
}