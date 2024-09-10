#pragma once

#include "Test.h"
#include "RenderObject.h"

#include <memory>

namespace test {
	class TestSimpleCube : public Test
	{
	public:
		TestSimpleCube();
		~TestSimpleCube();

		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnImGuiRender() override;
	private:
		std::unique_ptr<RenderObject> m_Cube;
	};
}