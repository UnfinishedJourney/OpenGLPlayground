#include "TestLights.h"
#include "Scene/Scene.h"
#include "Utilities/Logger.h"

#include <imgui.h>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>

TestLights::TestLights()
    : Test()
{
}

void TestLights::OnEnter()
{
    // Configure the scene's camera
    auto cam = std::make_shared<Camera>();
    m_Scene->SetCamera(cam);

    // Load a model into the scene
    if (!m_Scene->LoadModelIntoScene("pig", "simplelights", "Gold")) {
        Logger::GetLogger()->error("Failed to load 'pig' model in TestLights");
        return;
    }

    // Add a light
    LightData light1 = { glm::vec4(1.5f, 2.0f, 1.5f, 0.0f), glm::vec4(1.0f) };
    m_Scene->AddLight(light1);

    // Enable debug lights and grid
    m_Scene->SetBDebugLights(true);
    m_Scene->SetBGrid(true);

    // Modify the first material
    auto& materials = m_Scene->GetMaterials();
    if (!materials.empty()) {
        m_MatName = materials[0];
        auto existingMat = MaterialManager::GetInstance().GetMaterialByName(m_MatName);
        if (existingMat) {
            existingMat->AssignToPackedParams(MaterialParamType::Ambient, glm::vec3(0.9f, 0.1f, 0.3f));
        }
    }
}

void TestLights::OnExit()
{
    // Cleanup
    m_Renderer.reset();
    m_Scene->Clear();
}

void TestLights::OnUpdate(float /*deltaTime*/)
{
    // LOD or culling
    m_Scene->CullAndLODUpdate();
}

void TestLights::OnImGuiRender()
{
    ImGui::Begin("TestLights Controls");
    ImGui::Text("Use this panel for debugging or adjustments");
    ImGui::End();
}