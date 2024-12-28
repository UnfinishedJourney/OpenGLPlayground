#include "TestMenu.h"
#include "Test.h"
#include "imgui.h"
#include "Utilities/Logger.h"

TestMenu::TestMenu(TestManager& testManager)
    : m_TestManager(testManager)
{
}

void TestMenu::RegisterTest(const std::string& name, std::function<std::shared_ptr<Test>()> createFunc)
{
    m_Tests.emplace_back(name, createFunc);
    // Also register with the TestManager
    m_TestManager.RegisterTest(name, createFunc);
}

void TestMenu::OnImGuiRender()
{
    ImGui::Begin("Test Menu");
    for (auto& [testName, createFunc] : m_Tests) {
        if (ImGui::Button(testName.c_str())) {
            m_TestManager.SwitchTest(testName);
        }
    }
    ImGui::End();
}

// ------------------------------
//  TestMenuTest Implementation
// ------------------------------

TestMenuTest::TestMenuTest(std::weak_ptr<TestMenu> testMenu)
    : Test()
    , m_TestMenu(testMenu)
{
    Logger::GetLogger()->info("TestMenuTest created.");
}

TestMenuTest::~TestMenuTest()
{
    Logger::GetLogger()->info("TestMenuTest destroyed.");
}

void TestMenuTest::OnEnter()
{
    Logger::GetLogger()->info("TestMenuTest OnEnter called.");
    // Setup if needed
}

void TestMenuTest::OnExit()
{
    Logger::GetLogger()->info("TestMenuTest OnExit called.");
    // Cleanup
    m_Renderer.reset();
    if (m_Scene) {
        m_Scene->Clear();
    }
}

void TestMenuTest::OnUpdate(float /*deltaTime*/)
{
    // No special logic needed
}

void TestMenuTest::OnRender()
{
    // Usually no scene-based rendering for the menu
}

void TestMenuTest::OnImGuiRender()
{
    if (auto menu = m_TestMenu.lock()) {
        menu->OnImGuiRender();
    }
    else {
        Logger::GetLogger()->warn("TestMenuTest: The TestMenu no longer exists.");
    }
}

void TestMenuTest::OnWindowResize(int /*width*/, int /*height*/)
{
    // Typically no special handling needed
}

std::shared_ptr<Camera> TestMenuTest::GetCamera() const
{
    // No camera needed for the menu
    return nullptr;
}