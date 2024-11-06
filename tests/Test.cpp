#include "Test.h"
#include "imgui.h"

namespace test {

	TestMenu::TestMenu(std::shared_ptr<Test>& currentTest)
		: m_CurrentTest(currentTest)
	{
	}

	void TestMenu::OnImGuiRender()
	{
		for (const auto& test : m_Tests)
		{
			if (ImGui::Button(test.first.c_str()))
			{
				if (m_CurrentTest)
					m_CurrentTest->OnExit();
				m_CurrentTest = test.second();
				m_CurrentTest->OnEnter();
			}
		}
	}

}