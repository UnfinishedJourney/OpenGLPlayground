#include "TestLights.h"
#include "Resources/ModelManager.h"
#include "Resources/MaterialManager.h"
#include "Resources/ShaderManager.h"
#include "Scene/Transform.h"
#include "Scene/Lights.h"
#include "Scene/Screen.h"
#include "Utilities/Logger.h"
#include <imgui.h>

TestLights::TestLights() {
    // Initialization if needed
}

void TestLights::OnEnter() {

    auto& modelManager = ModelManager::GetInstance();
    auto& materialManager = MaterialManager::GetInstance();
    auto& shaderManager = ShaderManager::GetInstance();

    // Define mesh layout
    MeshLayout objMeshLayout = {
        true,  // Positions
        true,  // Normals
        false, // Tangents
        false, // Bitangents
        {}     // Texture Coordinates
    };

    // Get model
    auto model = modelManager.GetModel("pig");
    if (!model) {
        Logger::GetLogger()->error("Failed to load model 'pig'");
        return;
    }

    // Create material
    auto material = std::make_shared<Material>();

    // Gold material properties
    material->AddParam<glm::vec3>("material.Ka", glm::vec3(0.24725f, 0.1995f, 0.0745f));
    material->AddParam<glm::vec3>("material.Kd", glm::vec3(0.75164f, 0.60648f, 0.22648f));
    material->AddParam<glm::vec3>("material.Ks", glm::vec3(0.628281f, 0.555802f, 0.366065f));
    material->AddParam<float>("material.shininess", 51.2f);

    materialManager.AddMaterial("objMaterial", material);

    auto transform = std::make_shared<Transform>();
    transform->SetPosition(glm::vec3(0.0, 0.5, 0.0));

    // Get meshes from the model
    const auto& meshinfos = model->GetMeshesInfo();

    // Add render objects to the scene
    for (const auto& minfo : meshinfos) {
        auto renderObject = std::make_shared<RenderObject>(
            minfo.mesh,
            objMeshLayout,
            "objMaterial",
            "simplelights",
            transform
        );
        m_Scene->AddRenderObject(renderObject);
    }

    // Add light to the scene
    LightData light = { glm::vec4(1.5f, 2.0f, 1.5f, 0.0f) , glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) };
    m_Scene->AddLight(light);

    light = { glm::vec4(-1.5f, 2.0f, -1.5f, 0.0f) , glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) };
    m_Scene->AddLight(light);

    m_Scene->BuildBatches();
    m_Scene->SetBDebugLights(true);
    m_Scene->SetBGrid(true);
    //m_Scene->SetPostProcessingEffect(PostProcessingEffectType::EdgeDetection);
}

void TestLights::OnExit() {
    m_Renderer.reset();
    m_Scene->Clear();
}

void TestLights::OnUpdate(float deltaTime) {
    // Update objects or animations if needed
}


void TestLights::OnImGuiRender() {
    ImGui::Begin("TestLights Controls");
    ImGui::End();
}
