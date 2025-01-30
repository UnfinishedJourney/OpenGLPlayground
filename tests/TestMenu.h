#pragma once

#include <vector>
#include <string>
#include <functional>
#include <memory>
#include "TestManager.h"

/**
 * @class TestMenu
 * @brief A menu that displays available tests (registered with the TestManager) and allows switching between them.
 */
class TestMenu
{
public:
    /**
     * @brief Constructs a TestMenu that references a given TestManager.
     *
     * @param testManager Reference to the TestManager that will handle switching tests.
     */
    explicit TestMenu(TestManager& testManager);

    /**
     * @brief Registers a new test with the menu and with the TestManager.
     *
     * @param name       Name of the test.
     * @param createFunc Function/lambda that creates a shared_ptr<Test>.
     */
    void RegisterTest(const std::string& name, std::function<std::shared_ptr<Test>()> createFunc);

    /**
     * @brief Renders the ImGui interface for the menu, displaying buttons for each test.
     */
    void OnImGuiRender();

private:
    /**
     * @brief Reference to the main TestManager for switching between tests.
     */
    TestManager& m_TestManager;

    /**
     * @brief A local list of the registered tests (for display in the UI).
     */
    std::vector<std::pair<std::string, std::function<std::shared_ptr<Test>()>>> m_Tests;
};

/**
 * @class TestMenuTest
 * @brief A specialized Test that displays the TestMenu's ImGui interface as its primary content.
 */
class TestMenuTest : public Test
{
public:
    /**
     * @brief Constructs a TestMenuTest object that holds a weak_ptr to the TestMenu.
     *
     * @param testMenu A weak_ptr to the TestMenu we want to display.
     */
    explicit TestMenuTest(std::weak_ptr<TestMenu> testMenu);

    /**
     * @brief Logs destruction of the TestMenuTest.
     */
    ~TestMenuTest() override;

    /**
     * @brief Called when the TestMenuTest becomes the active test.
     */
    void OnEnter() override;

    /**
     * @brief Cleans up the internal resources when exiting.
     */
    void OnExit() override;

    /**
     * @brief Called every frame to update logic (currently empty for the menu).
     */
    void OnUpdate(float deltaTime) override;

    /**
     * @brief Called every frame to render (no scene-based rendering by default).
     */
    void OnRender() override;

    /**
     * @brief Renders the TestMenu's ImGui elements if the menu still exists.
     */
    void OnImGuiRender() override;

    /**
     * @brief Called when the window is resized (no special handling needed for a menu).
     */
    void OnWindowResize(int width, int height) override;

    /**
     * @brief Returns no camera, as the menu does not need one.
     *
     * @return Always returns nullptr.
     */
    std::shared_ptr<Camera> GetCamera() const override;

private:
    /**
     * @brief Weak reference to the TestMenu, to avoid circular ownership.
     */
    std::weak_ptr<TestMenu> m_TestMenu;
};