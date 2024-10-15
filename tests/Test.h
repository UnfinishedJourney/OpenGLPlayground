#pragma once
#include "Renderer/Renderer.h"

#include <vector>
#include <utility>
#include <memory>
#include <string>
#include <functional>
#include <vector>
#include <iostream>

namespace test {
	class Test
	{
	public:
		Test(std::shared_ptr<Renderer>& renderer)
			: m_Renderer(renderer)
		{}
		virtual ~Test() = default;

		virtual void OnUpdate(float deltaTime) {}
		virtual void OnRender() {}
		virtual void OnImGuiRender() {}
		void Clear() const
		{
			m_Renderer->Clear();
		}
	protected:
		std::shared_ptr<Renderer> m_Renderer;
	};

	class TestMenu : public Test
	{
	public:
		TestMenu(std::shared_ptr<Test>& currentTest, std::shared_ptr<Renderer>& renderer);
		void OnImGuiRender() override;

		template<typename T>
		void RegisterTest(const std::string& name)
		{
			std::cout << "Register test " << name << std::endl;
			m_Tests.push_back(std::make_pair(name, [this]() {return std::make_shared<T>(m_Renderer); }));
		}

	private:
		std::shared_ptr<Test>& m_CurrentTest;
		std::vector<std::pair<std::string, std::function<std::shared_ptr<Test>()>>> m_Tests;
	};
}