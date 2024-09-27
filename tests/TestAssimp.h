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
		MovingDuck(std::shared_ptr<MeshBuffer> meshComp, std::shared_ptr<Material> mat, std::unique_ptr<Transform> transform)
			: RenderObject(meshComp, mat, std::move(transform))
		{}

		void Update(float deltaTime) override {
			m_Transform->AddRotation(glm::vec3(0.0, deltaTime * 0.1, 0.0));
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
		//std::unique_ptr<ResourceManager> m_ResourceManager;
	};
}