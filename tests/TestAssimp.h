#pragma once

#include "Test.h"
#include "RenderObject.h"
#include "Model.h"
#include "ResourceManager.h"

#include <GLFW/glfw3.h>
#include <memory>

namespace test {

	class MovingDuck : public RenderObject {
	public:
		MovingDuck(std::shared_ptr<MeshComponent> meshComp, std::shared_ptr<Material> mat, std::unique_ptr<Transform> transform)
			: RenderObject(meshComp, mat, std::move(transform))
		{}

		void Update(float deltaTime) override {
			m_Transform->SetRotation(glm::vec4(0.5, 1.0, 0.0, (float)glfwGetTime() * 0.1f));
		}
	};

	class TestAssimp : public Test
	{
	public:
		TestAssimp();
		~TestAssimp();

		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnImGuiRender() override;
	private:
		std::unique_ptr<MovingDuck> m_DuckRenderObject;
		std::shared_ptr<Model> m_DuckModel;
		std::unique_ptr<ResourceManager> m_ResourceManager;
	};
}