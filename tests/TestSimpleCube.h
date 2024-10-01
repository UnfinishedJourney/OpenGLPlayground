#pragma once

#include "Test.h"
#include "Renderer/RenderObject.h"
#include "Resources/ResourceManager.h"

#include <GLFW/glfw3.h>
#include <memory>


namespace test {

	class MovingCube : public RenderObject {
	public:
		MovingCube(std::shared_ptr<MeshBuffer> meshComp, std::shared_ptr<Material> mat, std::unique_ptr<Transform> transform)
			: RenderObject(meshComp, mat, std::move(transform))
		{}

		void Update(float deltaTime) override {
			m_Transform->SetRotation(glm::vec4(0.5, 1.0, 0.0, 0.5 + (float)glfwGetTime() * 0.1f));
		}
	};


	class TestSimpleCube : public Test
	{
	public:
		TestSimpleCube();
		~TestSimpleCube();

		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnImGuiRender() override;
	private:
		std::unique_ptr<MovingCube> m_Cube;
		//std::unique_ptr<ResourceManager> m_ResourceManager;
	};
}