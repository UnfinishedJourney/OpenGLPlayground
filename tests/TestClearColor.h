#pragma once
#include "Test.h"

class TestClearColor : public Test
{
public:
    TestClearColor();
    ~TestClearColor() override = default;
    void OnRender() override;
    void OnImGuiRender() override;

private:
    float m_ClearColor[4];
};

