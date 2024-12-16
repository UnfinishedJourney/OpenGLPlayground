#pragma once
#include "Test.h"
#include "Renderer/Renderer.h"
#include "Scene/Scene.h"  // not SceneGraph
#include "Graphics/Meshes/ModelLoader.h"
#include "Resources/MaterialManager.h"
#include "Resources/ShaderManager.h"

class TestLights : public Test {
public:
    TestLights();
    ~TestLights() override = default;

    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float deltaTime) override;
    void OnImGuiRender() override;

private:
};