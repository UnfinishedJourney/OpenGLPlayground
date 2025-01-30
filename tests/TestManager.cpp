#include "TestManager.h"
#include "Utilities/Logger.h"

TestManager::TestManager()
    : m_CurrentTest(nullptr)
{
    // Constructor body: no specialized logic.
}

TestManager::~TestManager()
{
    // Ensure the current test properly exits before destruction.
    if (m_CurrentTest)
    {
        m_CurrentTest->OnExit();
    }
}

void TestManager::RegisterTest(const std::string& name, std::function<std::shared_ptr<Test>()> createFunc)
{
    m_TestRegistry[name] = createFunc;
    // Optionally log that a new test was registered
    Logger::GetLogger()->info("TestManager: Registered test '{}'.", name);
}

void TestManager::SwitchTest(const std::string& testName)
{
    auto it = m_TestRegistry.find(testName);
    if (it != m_TestRegistry.end())
    {
        // Exit the current test if it exists.
        if (m_CurrentTest)
        {
            m_CurrentTest->OnExit();
        }

        // Create and enter the new test.
        m_CurrentTest = it->second();
        m_CurrentTest->OnEnter();

        Logger::GetLogger()->info("TestManager: Switched to test '{}'.", testName);
    }
    else
    {
        Logger::GetLogger()->error("TestManager: Test '{}' not found in the registry.", testName);
    }
}

void TestManager::UpdateCurrentTest(float deltaTime)
{
    if (m_CurrentTest)
    {
        m_CurrentTest->OnUpdate(deltaTime);
    }
}

void TestManager::RenderCurrentTest()
{
    if (m_CurrentTest)
    {
        m_CurrentTest->OnRender();
    }
}

void TestManager::RenderCurrentTestImGui()
{
    if (m_CurrentTest)
    {
        m_CurrentTest->OnImGuiRender();
    }
}

void TestManager::HandleWindowResize(int width, int height)
{
    if (m_CurrentTest)
    {
        m_CurrentTest->OnWindowResize(width, height);
    }
}

std::shared_ptr<Camera> TestManager::GetCurrentCamera() const
{
    // Forward the call to the current test's camera.
    return m_CurrentTest ? m_CurrentTest->GetCamera() : nullptr;
}