#include "Test.h"
#include "imgui.h"

namespace test {

	TestMenu::TestMenu(std::shared_ptr<Test>& currentTest, std::shared_ptr<Renderer>& renderer)
		: Test(renderer), m_CurrentTest(currentTest)
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

