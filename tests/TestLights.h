#pragma once

#include "Test.h"
#include "ResourceManager.h"
#include "RenderObject.h"
#include "Scene.h"

#include <GLFW/glfw3.h>
#include <memory>

namespace test {

	class LightObject : public RenderObject {
	public:
		LightObject(std::shared_ptr<MeshBuffer> meshComp, std::shared_ptr<Material> mat, std::unique_ptr<Transform> transform)
			: RenderObject(meshComp, mat, std::move(transform))
		{}
		virtual void Render() override;
	};

	class TestLights : public Test
	{
	public:
		TestLights();

		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnImGuiRender() override;
	private:
		std::unique_ptr<Scene> m_Scene;
		std::unique_ptr<ResourceManager> m_ResourceManager;
	};
}