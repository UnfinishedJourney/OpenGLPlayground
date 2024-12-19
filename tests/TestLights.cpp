#include "TestLights.h"
#include "Scene/Scene.h"
#include "Utilities/Logger.h"
#include <imgui.h>

TestLights::TestLights() : Test() {}

void TestLights::OnEnter() {
    auto cam = std::make_shared<Camera>();
    m_Scene->SetCamera(cam);

    MeshLayout layout = { true, true, false, false, {} };
    if (!m_Scene->LoadModelIntoScene("pig", "simplelights", "objMaterial", layout)) {
        Logger::GetLogger()->error("Failed to load pig model");
        return;
    }

    LightData light1 = { glm::vec4(1.5f,2.0f,1.5f,0.0f), glm::vec4(1.0f) };
    m_Scene->AddLight(light1);

    LightData light2 = { glm::vec4(-1.5f,2.0f,-1.5f,0.0f), glm::vec4(1.0f,0.0f,0.0f,1.0f) };
    m_Scene->AddLight(light2);

    m_Scene->SetBDebugLights(true);
    m_Scene->SetBGrid(true);
}

void TestLights::OnExit() {
    m_Renderer.reset();
    m_Scene->Clear();
}

void TestLights::OnUpdate(float deltaTime) {
    m_Scene->CullAndLODUpdate();
}

void TestLights::OnImGuiRender() {
    ImGui::Begin("TestLights Controls");
    ImGui::Text("Use this panel for debugging or adjustments");
    ImGui::End();
}