#include "TestManager.h"
#include "Utilities/Logger.h"

TestManager::TestManager()
    : m_CurrentTest(nullptr) {
}

TestManager::~TestManager() {
    if (m_CurrentTest) {
        m_CurrentTest->OnExit();
    }
}

void TestManager::RegisterTest(const std::string& name, std::function<std::shared_ptr<Test>()> createFunc) {
    m_TestRegistry[name] = createFunc;
}

void TestManager::SwitchTest(const std::string& testName) {
    auto it = m_TestRegistry.find(testName);
    if (it != m_TestRegistry.end()) {
        if (m_CurrentTest) {
            m_CurrentTest->OnExit();
        }
        m_CurrentTest = it->second();
        m_CurrentTest->OnEnter();
        Logger::GetLogger()->info("Switched to test '{}'.", testName);
    }
    else {
        Logger::GetLogger()->error("Test '{}' not found.", testName);
    }
}

void TestManager::UpdateCurrentTest(float deltaTime) {
    if (m_CurrentTest) {
        m_CurrentTest->OnUpdate(deltaTime);
    }
}

void TestManager::RenderCurrentTest() {
    if (m_CurrentTest) {
        m_CurrentTest->OnRender();
    }
}

void TestManager::RenderCurrentTestImGui() {
    if (m_CurrentTest) {
        m_CurrentTest->OnImGuiRender();
    }
}

void TestManager::HandleWindowResize(int width, int height) {
    if (m_CurrentTest) {
        m_CurrentTest->OnWindowResize(width, height);
    }
}

std::shared_ptr<Camera> TestManager::GetCurrentCamera() const {
    if (m_CurrentTest) {
        return m_CurrentTest->GetCamera();
    }
    return nullptr;
}