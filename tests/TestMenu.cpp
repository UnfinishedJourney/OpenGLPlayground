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

TestMenuTest::TestMenuTest(TestMenu& testMenu)
    : m_TestMenu(testMenu) {
}

void TestMenuTest::OnEnter() {
    // No initialization needed for the test menu
}

void TestMenuTest::OnExit() {
    // No cleanup needed for the test menu
}

void TestMenuTest::OnUpdate(float deltaTime) {
    // No update logic required
}

void TestMenuTest::OnRender() {
    // No rendering logic required
}

void TestMenuTest::OnImGuiRender() {
    m_TestMenu.OnImGuiRender();
}

void TestMenuTest::OnWindowResize(int width, int height) {
    // No action needed for window resize
}

std::shared_ptr<Camera> TestMenuTest::GetCamera() const {
    return nullptr;
}
