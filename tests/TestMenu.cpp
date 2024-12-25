#include "TestMenu.h"
#include "imgui.h"

TestMenu::TestMenu(TestManager& testManager)
    : m_TestManager(testManager) {
}

void TestMenu::RegisterTest(const std::string& name, std::function<std::shared_ptr<Test>()> createFunc) {
    m_Tests.emplace_back(name, createFunc);
    m_TestManager.RegisterTest(name, createFunc);
}

void TestMenu::OnImGuiRender() {
    ImGui::Begin("Test Menu");
    for (const auto& test : m_Tests) {
        if (ImGui::Button(test.first.c_str())) {
            m_TestManager.SwitchTest(test.first);
        }
    }
    ImGui::End();
}

TestMenuTest::TestMenuTest(std::weak_ptr<TestMenu> testMenu)
    : Test(), m_TestMenu(testMenu) {
    Logger::GetLogger()->info("TestMenuTest created.");
}

void TestMenuTest::OnEnter() {
    Logger::GetLogger()->info("TestMenuTest OnEnter.");
    // Additional initialization if needed
}

void TestMenuTest::OnExit() {
    Logger::GetLogger()->info("TestMenuTest OnExit.");
    m_Renderer.reset();
    if (m_Scene) {
        m_Scene->Clear();
    }
}

void TestMenuTest::OnUpdate(float deltaTime) {
    // Update logic if needed
}

void TestMenuTest::OnRender() {
    // Rendering logic if needed
}

void TestMenuTest::OnImGuiRender() {
    if (auto testMenu = m_TestMenu.lock()) { // Safely access TestMenu
        testMenu->OnImGuiRender();
    }
    else {
        Logger::GetLogger()->warn("TestMenuTest: TestMenu no longer exists.");
    }
}

void TestMenuTest::OnWindowResize(int width, int height) {
    // Handle window resize if needed
}

std::shared_ptr<Camera> TestMenuTest::GetCamera() const {
    return nullptr;
}

TestMenuTest::~TestMenuTest() {
    Logger::GetLogger()->info("TestMenuTest destroyed.");
}