#pragma once
#include "Test.h"
#include "Renderer/Renderer.h"
#include "Resources/MaterialManager.h"
#include "Resources/ShaderManager.h"

class TestShadows : public Test {
public:
    TestShadows();
    ~TestShadows() override = default;

    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float deltaTime) override;
    void OnImGuiRender() override;

private:
    int m_MatID;
};