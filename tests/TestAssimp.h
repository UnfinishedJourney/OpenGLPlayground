#pragma once

#include "Test.h"
#include "Model.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"

#include <memory>

namespace test {
	class TestAssimp : public Test
	{
	public:
		TestAssimp();
		~TestAssimp();

		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnImGuiRender() override;
	private:
		std::unique_ptr<Model> m_Model;
	};
}