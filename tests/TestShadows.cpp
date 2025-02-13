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
    std::string shaderName = "simpleLightsShadowed";

    // Load a model into the scene
    if (!scene_->LoadStaticModelIntoScene("pig", shaderName)) {
        Logger::GetLogger()->error("Failed to load 'pig' model in TestShadow");
        return;
    }

    auto& materialManager = graphics::MaterialManager::GetInstance();

    auto& resourceManager = ResourceManager::GetInstance();
    auto [meshLayout, matLayout] = resourceManager.GetLayoutsFromShader(shaderName);

    {
        auto floorMat = std::make_unique<graphics::Material>(matLayout);
        floorMat->SetName("floorMat");

        floorMat->AssignToPackedParams(MaterialParamType::Ambient, glm::vec3(0.0, 0.0, 1.0));
        floorMat->AssignToPackedParams(MaterialParamType::Diffuse, glm::vec3(0.0, 1.0, 0.0));
        floorMat->AssignToPackedParams(MaterialParamType::Specular, glm::vec3(1.0, 0.0, 0.0));
        floorMat->AssignToPackedParams(MaterialParamType::Shininess, 100.0f);

        graphics::MaterialManager::GetInstance().AddMaterial(std::move(floorMat));
        int floorMatID = graphics::MaterialManager::GetInstance().GetMaterialIDByName("floorMat").value();

        if (!scene_->LoadPrimitiveIntoScene("floor", shaderName, floorMatID)) {
            Logger::GetLogger()->error("Failed to load cube primitive.");
        }
    }


    // Add a light
    //LightData light1 = { glm::vec4(1.5f, 2.0f, 1.5f, 1.0f), glm::vec4(1.0f) };
    LightData light1 = { glm::vec4(-1.0f, -1.0f, 0.0f, 0.0f), glm::vec4(1.0f) };

    auto lightManager = scene_->GetLightManager();
    lightManager->AddLight(light1);

    scene_->SetShowDebugLights(true);
    scene_->SetShowShadows(true);
    auto& materials = graphics::MaterialManager::GetInstance().GetMaterials();
    if (materials.size() > 0) {
        materials[0]->AssignToPackedParams(MaterialParamType::Ambient, glm::vec3(0.9f, 0.1f, 0.3f));
    }

    scene_->BuildStaticBatchesIfNeeded();
}

void TestShadows::OnExit()
{
    // Cleanup
    renderer_.reset();
    scene_->Clear();
}

void TestShadows::OnUpdate(float /*deltaTime*/)
{
    // LOD or culling
    scene_->CullAndLODUpdate();
}

void TestShadows::OnImGuiRender()
{
    ImGui::Begin("TestShadow Controls");
    ImGui::Text("Use this panel for debugging or adjustments");
    ImGui::End();
}