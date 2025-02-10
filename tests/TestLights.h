#pragma once
#include "Test.h"
#include "Renderer/Renderer.h"
#include "Graphics/Materials/MaterialManager.h"
#include "Graphics/Shaders/ShaderManager.h"

class TestLights : public Test {
public:
    TestLights();
    ~TestLights() override = default;

    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float deltaTime) override;
    void OnImGuiRender() override;

private:
    int m_MatID;
};