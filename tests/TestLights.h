#pragma once

#include "Test.h"
#include "Renderer/RenderObject.h"
#include "Resources/ResourceManager.h"

#include <GLFW/glfw3.h>
#include <memory>
namespace test {

	class LightObject : public RenderObject {
	public:
		LightObject(std::shared_ptr<MeshBuffer> meshBuffer, MaterialName materialName, ShaderName shaderName, std::shared_ptr<Transform> transform)
			: RenderObject(std::move(meshBuffer), materialName, shaderName, std::move(transform))
		{}
	};

	class TestLights : public Test
	{
	public:
		TestLights(std::shared_ptr<Renderer>& renderer);
		//~TestLight() override = default;

		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnImGuiRender() override;
	private:
		std::vector<std::shared_ptr<LightObject>> m_Pig;
	};
}