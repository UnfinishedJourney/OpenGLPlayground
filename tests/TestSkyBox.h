#pragma once

#include "Test.h"
#include "Shader.h"
#include "Texture.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"
#include "SkyBox.h"

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"

#include <memory>

namespace test {
	class TestSkyBox : public Test
	{
	public:
		TestSkyBox();
		~TestSkyBox();

		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnImGuiRender() override;
	private:
		std::unique_ptr<SkyBox> m_SkyBox;
		std::unique_ptr<Shader> m_Shader;
		GLuint m_TexID;

		glm::mat4 m_Model;
	};
}