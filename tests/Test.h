#pragma once
#include "Renderer/Renderer.h"

#include <vector>
#include <utility>
#include <string>
#include <functional>
#include <vector>
#include <iostream>

namespace test {
	class Test
	{
	public:
		Test() {}
		virtual ~Test() {}

		virtual void OnUpdate(float deltaTime) {}
		virtual void OnRender() {}
		virtual void OnImGuiRender() {}
		static void InitializeRenderer() {
			//if (!s_ResourceManager) {
			//	//s_ResourceManager = std::make_unique<ResourceManager>(shaderMetadataPath);
			//	s_ResourceManager = std::make_unique<ResourceManager>();
			//}
			if (!s_Renderer)
			{
				s_Renderer = std::make_unique<Renderer>();
			}
		}
	protected:
		static std::unique_ptr<Renderer> s_Renderer;
	};

	class TestMenu : public Test
	{
	public:
		TestMenu(std::shared_ptr<Test>& currentTest);
		void OnImGuiRender() override;

		template<typename T>
		void RegisterTest(const std::string& name)
		{
			std::cout << "Register test " << name << std::endl;
			m_Tests.push_back(std::make_pair(name, []() {return std::make_shared<T>(); }));
		}

	private:
		std::shared_ptr<Test>& m_CurrentTest;
		std::vector<std::pair<std::string, std::function<std::shared_ptr<Test>()>>> m_Tests;
	};
}