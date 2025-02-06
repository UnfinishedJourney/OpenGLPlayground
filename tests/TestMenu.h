#pragma once
#include <vector>
#include <string>
#include <functional>
#include <memory>
#include "TestManager.h"

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

#include "Test.h"


class TestMenuTest : public Test
{
public:
    TestMenuTest(std::weak_ptr<TestMenu> menuPtr);
    ~TestMenuTest() override;

    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float dt) override;
    void OnRender() override;
    void OnImGuiRender() override;

private:
    std::weak_ptr<TestMenu> m_Menu;
};