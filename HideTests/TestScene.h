#pragma once

#include "Test.h"
#include "Scene/Scene.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"

#include <memory>

namespace test {
	class TestScene : public Test
	{
	public:
		TestScene();
		~TestScene();

		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnImGuiRender() override;
	private:
		std::unique_ptr<Scene> m_Scene;
	};
}