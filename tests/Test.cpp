#include "Test.h"
#include "imgui.h"

namespace test {
	std::unique_ptr<Renderer> Test::s_Renderer = nullptr;

	TestMenu::TestMenu(std::shared_ptr<Test>& currentTest)
		: m_CurrentTest(currentTest)
	{
	}

	void TestMenu::OnImGuiRender()
	{
		for (auto& test : m_Tests) 
		{
			if (ImGui::Button(test.first.c_str()))
				m_CurrentTest = test.second();
		}
	}
}

