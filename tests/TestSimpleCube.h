#pragma once

#include "Test.h"
#include "Renderer/RenderObject.h"
#include "Resources/ResourceManager.h"

#include <GLFW/glfw3.h>
#include <memory>


namespace test {

	class MovingCube : public RenderObject {
	public:
		MovingCube(const std::shared_ptr<Mesh>& mesh,
			const MeshLayout& meshLayout,
			const std::string& materialName,
			const std::string& shaderName,
			const std::shared_ptr<Transform>& transform)
			: RenderObject(mesh, meshLayout, materialName, shaderName, transform)
		{}

		void Update(float deltaTime) override {
			m_Transform->SetRotation(glm::vec3(0.0, 0.0 + (float)glfwGetTime() * 1.0f, 0.0));
		}
	};


	class TestSimpleCube : public Test
	{
	public:
		TestSimpleCube(std::shared_ptr<Renderer>& renderer);
		~TestSimpleCube() override = default;

		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnImGuiRender() override;
	private:
		std::shared_ptr<MovingCube> m_Cube;
	};
}