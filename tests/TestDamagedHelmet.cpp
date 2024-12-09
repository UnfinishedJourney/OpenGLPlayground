#include "TestDamagedHelmet.h"
#include "Resources/ModelManager.h"
#include "Resources/MaterialManager.h"
#include "Resources/ShaderManager.h"
#include "Resources/TextureManager.h"
#include "Scene/Transform.h"
#include "Scene/Lights.h"
#include "Scene/Screen.h"
#include "Utilities/Logger.h"
#include <imgui.h>

TestDamagedHelmet::TestDamagedHelmet() {
    // Initialization if needed
}

void TestDamagedHelmet::OnEnter() {

    auto& modelManager = ModelManager::GetInstance();
    auto& materialManager = MaterialManager::GetInstance();
    auto& shaderManager = ShaderManager::GetInstance();
    auto& textureManager = TextureManager::GetInstance();

    // Define mesh layout
    MeshLayout objMeshLayout = {
        true,  // Positions
        true,  // Normals
        true, // Tangents
        false, // Bitangents
        {TextureType::Albedo}     // Texture Coordinates
    };

    // Get model
    auto model = modelManager.GetModel("helmet");
    if (!model) {
        Logger::GetLogger()->error("Failed to load model 'helmet'");
        return;
    }

    // Create material
    auto material = std::make_shared<Material>();


    std::shared_ptr<Texture2D> brdfLUT = textureManager.GetTexture2D("brdfLUT");;

    std::shared_ptr<Texture2D> albedoTexture = model->GetTexture(0, TextureType::Albedo);
    std::shared_ptr<Texture2D> normalTexture = model->GetTexture(0, TextureType::Normal);
    std::shared_ptr<Texture2D> occlusionTexture = model->GetTexture(0, TextureType::Occlusion);
    std::shared_ptr<Texture2D> roughmetTexture = model->GetTexture(0, TextureType::RoughnessMetallic);
    std::shared_ptr<Texture2D> emissiveTexture = model->GetTexture(0, TextureType::Emissive);


    material->AddTexture(brdfLUT, 0);
    material->AddTexture(albedoTexture, 1);
    material->AddTexture(normalTexture, 2);
    material->AddTexture(roughmetTexture, 3);
    material->AddTexture(occlusionTexture, 4);
    material->AddTexture(emissiveTexture, 5);

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
            "HelmetPBR",
            transform
        );
        m_Scene->AddRenderObject(renderObject);
    }

    // Add light to the scene
    LightData light = { glm::vec4(10.0f, 10.0f, 10.0f, 0.0f) , glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) };
    m_Scene->AddLight(light);

    m_Scene->BuildBatches();
    m_Scene->SetBDebugLights(true);
    m_Scene->SetBGrid(true);
}

void TestDamagedHelmet::OnExit() {
    m_Renderer.reset();
    m_Scene->Clear();
}

void TestDamagedHelmet::OnUpdate(float deltaTime) {
    // Update objects or animations if needed
}


void TestDamagedHelmet::OnImGuiRender() {
    ImGui::Begin("TestDamagedHelmet Controls");
    ImGui::End();
}
