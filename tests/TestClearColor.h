#pragma once
#include "Test.h"

class TestClearColor : public Test
{
public:
    TestClearColor();
    ~TestClearColor() override = default;

    void OnEnter() override {};
    void OnExit() override {};
    void OnUpdate(float deltaTime) override;
    void OnImGuiRender() override;

private:
    float m_ClearColor[4];
};

