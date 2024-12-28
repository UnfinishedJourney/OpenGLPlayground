#pragma once

#include <vector>
#include <string>
#include <functional>
#include <memory>

#include "TestManager.h"

/**
 * @brief Menu for registering available tests and displaying them in ImGui.
 */
class TestMenu
{
public:
    explicit TestMenu(TestManager& testManager);

    void RegisterTest(const std::string& name, std::function<std::shared_ptr<Test>()> createFunc);
    void OnImGuiRender();

private:
    TestManager& m_TestManager;
    std::vector<std::pair<std::string, std::function<std::shared_ptr<Test>()>>> m_Tests;
};

/**
 * @brief A specialized Test that displays the TestMenu (hence letting user select a different test).
 */
class TestMenuTest : public Test
{
public:
    explicit TestMenuTest(std::weak_ptr<TestMenu> testMenu);
    ~TestMenuTest() override;

    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float deltaTime) override;
    void OnRender() override;
    void OnImGuiRender() override;
    void OnWindowResize(int width, int height) override;
    std::shared_ptr<Camera> GetCamera() const override;

private:
    std::weak_ptr<TestMenu> m_TestMenu;
};