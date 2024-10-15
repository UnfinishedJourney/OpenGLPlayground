#pragma once

#include "Test.h"
#include "Renderer/RenderObject.h"
#include "Resources/ResourceManager.h"

#include <GLFW/glfw3.h>
#include <memory>


namespace test {

	class TestSkyBox : public Test
	{
	public:
		TestSkyBox(std::shared_ptr<Renderer>& renderer);
		~TestSkyBox() override = default;

		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnImGuiRender() override;
	private:
		std::shared_ptr<MeshBuffer> m_MeshComponent;
	};
}