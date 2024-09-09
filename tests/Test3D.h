#pragma once

#include "Test.h"
#include "Shader.h"
#include "Texture.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"
#include "Cube.h"

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"

#include <memory>

namespace test {
	class Test3D : public Test
	{
	public:
		Test3D();
		~Test3D();

		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnImGuiRender() override;
	private:
		std::unique_ptr<Mesh> m_Cube;
		std::unique_ptr<Shader> m_Shader;

		glm::mat4 m_Proj;
		glm::mat4 m_View;
		glm::mat4 m_Model;
	};
}