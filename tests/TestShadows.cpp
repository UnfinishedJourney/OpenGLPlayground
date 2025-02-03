#include "TestShadows.h"
#include "Scene/Scene.h"
#include "Utilities/Logger.h"

#include <imgui.h>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>

TestShadows::TestShadows()
    : Test()
{
}

void TestShadows::OnEnter()
{
    // Configure the scene's camera
    auto cam = std::make_shared<Camera>();
    m_Scene->SetCamera(cam);

    // Load a model into the scene
    if (!m_Scene->LoadStaticModelIntoScene("pig", "simplelights")) {
        Logger::GetLogger()->error("Failed to load 'pig' model in TestShadow");
        return;
    }

    // Add a light
    LightData light1 = { glm::vec4(1.5f, 2.0f, 1.5f, 0.0f), glm::vec4(1.0f) };
    m_Scene->AddLight(light1);

    // Enable debug lights and grid
    m_Scene->SetShowGrid(true);
    m_Scene->SetShowDebugLights(true);
    m_Scene->SetShowShadows(true);
    auto existingMat = MaterialManager::GetInstance().GetMaterialByID(0);
    if (existingMat) {
        existingMat->AssignToPackedParams(MaterialParamType::Ambient, glm::vec3(0.9f, 0.1f, 0.3f));
    }
}

void TestShadows::OnExit()
{
    // Cleanup
    m_Renderer.reset();
    m_Scene->Clear();
}

void TestShadows::OnUpdate(float /*deltaTime*/)
{
    // LOD or culling
    m_Scene->CullAndLODUpdate();
}

void TestShadows::OnImGuiRender()
{
    ImGui::Begin("TestShadow Controls");
    ImGui::Text("Use this panel for debugging or adjustments");
    ImGui::End();
}