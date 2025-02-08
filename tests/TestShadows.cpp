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

    std::string shaderName = "simpleLightsShadowed";

    // Load a model into the scene
    if (!m_Scene->LoadStaticModelIntoScene("pig", shaderName)) {
        Logger::GetLogger()->error("Failed to load 'pig' model in TestShadow");
        return;
    }

    auto& materialManager = MaterialManager::GetInstance();

    auto& resourceManager = ResourceManager::GetInstance();
    auto [meshLayout, matLayout] = resourceManager.getLayoutsFromShader(shaderName);

    auto floorMat = std::make_shared<Material>(matLayout);
    floorMat->SetName("floorMat");

    floorMat->AssignToPackedParams(MaterialParamType::Ambient, glm::vec3(0.0, 0.0, 1.0));
    floorMat->AssignToPackedParams(MaterialParamType::Diffuse, glm::vec3(0.0, 1.0, 0.0));
    floorMat->AssignToPackedParams(MaterialParamType::Specular, glm::vec3(1.0, 0.0, 0.0));
    floorMat->AssignToPackedParams(MaterialParamType::Shininess, 100.0f);

    MaterialManager::GetInstance().AddMaterial(floorMat);

    if (!m_Scene->LoadPrimitiveIntoScene("floor", shaderName, floorMat->GetID())) {
        Logger::GetLogger()->error("Failed to load cube primitive.");
    }

    // Add a light
    //LightData light1 = { glm::vec4(1.5f, 2.0f, 1.5f, 1.0f), glm::vec4(1.0f) };
    LightData light1 = { glm::vec4(-1.0f, -1.0f, 0.0f, 0.0f), glm::vec4(1.0f) };

    auto lightManager = m_Scene->GetLightManager();
    lightManager->AddLight(light1);

    m_Scene->SetShowDebugLights(true);
    m_Scene->SetShowShadows(true);
    auto existingMat = MaterialManager::GetInstance().GetMaterialByID(1);
    if (existingMat) {
        existingMat->AssignToPackedParams(MaterialParamType::Ambient, glm::vec3(0.4f, 0.5f, 0.8f));
    }

    m_Scene->BuildStaticBatchesIfNeeded();
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