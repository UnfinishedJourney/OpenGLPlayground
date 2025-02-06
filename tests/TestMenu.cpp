#include "TestMenu.h"
#include "imgui.h"
#include "Utilities/Logger.h"

TestMenu::TestMenu(TestManager& testManager)
    : m_TestManager(testManager)
{}

void TestMenu::RegisterTest(const std::string& name, std::function<std::shared_ptr<Test>()> createFunc)
{
    m_Tests.emplace_back(name, createFunc);
    m_TestManager.RegisterTest(name, createFunc);

    Logger::GetLogger()->info("TestMenu: Registered test '{}'.", name);
}

void TestMenu::OnImGuiRender()
{
    ImGui::Begin("Test Menu");
    for (auto& [testName, createFunc] : m_Tests)
    {
        if (ImGui::Button(testName.c_str()))
        {
            m_TestManager.SwitchTest(testName);
        }
    }
    ImGui::End();
}


TestMenuTest::TestMenuTest(std::weak_ptr<TestMenu> menuPtr)
    : Test()
    , m_Menu(menuPtr)
{
    Logger::GetLogger()->info("TestMenuTest created.");
}

TestMenuTest::~TestMenuTest()
{
    Logger::GetLogger()->info("TestMenuTest destructor called.");
    // Do NOT call OnExit() here.
}

void TestMenuTest::OnEnter()
{
    Logger::GetLogger()->info("TestMenuTest OnEnter.");
}

void TestMenuTest::OnExit()
{
    Logger::GetLogger()->info("TestMenuTest OnExit.");

    // Then do the base cleanup (scene->Clear, renderer.reset)
    Test::OnExit();
}

void TestMenuTest::OnUpdate(float /*dt*/)
{
    // no updates needed
}

void TestMenuTest::OnRender()
{
    // typically no scene
}

void TestMenuTest::OnImGuiRender()
{
    if (auto menu = m_Menu.lock())
    {
        menu->OnImGuiRender();
    }
    else
    {
        Logger::GetLogger()->warn("TestMenuTest: menu pointer is expired.");
    }
}