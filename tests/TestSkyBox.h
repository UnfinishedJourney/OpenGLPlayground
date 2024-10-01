#pragma once

#include "Test.h"
#include "Graphics/Shaders/Shader.h"
#include "Graphics/Textures/TextureSkyBox.h"
#include "Graphics/Buffers/VertexBuffer.h"
#include "Graphics/Buffers/VertexBufferLayout.h"
#include "Graphics/Buffers/IndexBuffer.h"
#include "Graphics/Meshes/SkyBox.h"

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

		glm::mat4 m_Model;
	};
}