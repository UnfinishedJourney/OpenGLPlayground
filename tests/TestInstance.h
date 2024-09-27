#pragma once

#include "Test.h"
#include "RenderObject.h"
#include "ResourceManager.h"

#include <GLFW/glfw3.h>
#include <memory>


namespace test {

	class TestInstance : public Test
	{
	public:
		TestInstance();
		~TestInstance();

		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnImGuiRender() override;
	private:
		std::unique_ptr<InstancedRenderObject> m_Instances;
		//std::unique_ptr<ResourceManager> m_ResourceManager;
	};
}