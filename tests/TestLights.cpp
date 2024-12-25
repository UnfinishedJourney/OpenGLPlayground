#include "TestLights.h"
#include "Scene/Scene.h"
#include "Utilities/Logger.h"
#include <imgui.h>

TestLights::TestLights() : Test() {}

void TestLights::OnEnter() {
    auto cam = std::make_shared<Camera>();
    m_Scene->SetCamera(cam);

    if (!m_Scene->LoadModelIntoScene("pig", "simplelights", "Gold")) {
        Logger::GetLogger()->error("Failed to load pig model");
        return;
    }

    LightData light1 = { glm::vec4(1.5f,2.0f,1.5f,0.0f), glm::vec4(1.0f) };
    m_Scene->AddLight(light1);

    //LightData light2 = { glm::vec4(-1.5f,2.0f,-1.5f,0.0f), glm::vec4(1.0f,0.0f,0.0f,1.0f) };
    //LightData light2 = { glm::vec4(-1.5f,2.0f,-1.5f,0.0f), glm::vec4(0.1f) };
    //m_Scene->AddLight(light2);

    m_Scene->SetBDebugLights(true);
    m_Scene->SetBGrid(true);

    auto& materials = m_Scene->GetMaterials();
    m_MatName = materials[0];
    auto existingMat = MaterialManager::GetInstance().GetMaterialByName(m_MatName);
    existingMat->SetParam(MaterialParamType::Ambient, glm::vec3(0.9, 0.1, 0.3));
}

void TestLights::OnExit() {
    m_Renderer.reset();
    m_Scene->Clear();
}

void TestLights::OnUpdate(float deltaTime) {
    m_Scene->CullAndLODUpdate();
}

void TestLights::OnImGuiRender() {
    //    ImGui::Begin("TestBistro Controls");
//    glm::vec3& position = m_Camera->GetPositionRef();
//    ImGui::SliderFloat3("Camera Position", &position.x, 0.0f, 2000.0f);
//    ImGui::End();
    ImGui::Begin("TestLights Controls");
    ImGui::Text("Use this panel for debugging or adjustments");
    ImGui::End();
}