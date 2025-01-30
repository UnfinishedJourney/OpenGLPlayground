#include "TestMenu.h"
#include "Test.h"
#include "imgui.h"
#include "Utilities/Logger.h"

TestMenu::TestMenu(TestManager& testManager)
    : m_TestManager(testManager)
{
    // Constructor body
}

void TestMenu::RegisterTest(const std::string& name, std::function<std::shared_ptr<Test>()> createFunc)
{
    // Store locally for UI display
    m_Tests.emplace_back(name, createFunc);

    // Also register with the TestManager
    m_TestManager.RegisterTest(name, createFunc);

    Logger::GetLogger()->info("TestMenu: Registered '{}' with both the menu and TestManager.", name);
}

void TestMenu::OnImGuiRender()
{
    ImGui::Begin("Test Menu");
    ImGui::Text("Available Tests:");
    // Display a button for each test in our local list
    for (auto& [testName, createFunc] : m_Tests)
    {
        if (ImGui::Button(testName.c_str()))
        {
            m_TestManager.SwitchTest(testName);
        }
    }
    ImGui::End();
}

// -----------------------------
//     TestMenuTest
// -----------------------------

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
    // Any additional setup needed when we switch to the menu test
}

void TestMenuTest::OnExit()
{
    Logger::GetLogger()->info("TestMenuTest OnExit called.");
    // Use the parent Test's cleanup routine
    Test::OnExit();
}

void TestMenuTest::OnUpdate(float /*deltaTime*/)
{
    // No scene or gameplay logic for a simple menu
}

void TestMenuTest::OnRender()
{
    // Typically no 3D/scene-based rendering for the menu
}

void TestMenuTest::OnImGuiRender()
{
    if (auto menu = m_TestMenu.lock())
    {
        // Render the test menu's UI if it still exists
        menu->OnImGuiRender();
    }
    else
    {
        Logger::GetLogger()->warn("TestMenuTest: The TestMenu no longer exists.");
    }
}

void TestMenuTest::OnWindowResize(int /*width*/, int /*height*/)
{
    // Menu doesn't need a special resize routine
}

std::shared_ptr<Camera> TestMenuTest::GetCamera() const
{
    // No camera needed for the menu test
    return nullptr;
}