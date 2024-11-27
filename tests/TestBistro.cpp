#include "TestBistro.h"
#include "Resources/ModelManager.h"
#include "Resources/MaterialManager.h"
#include "Resources/ShaderManager.h"
#include "Scene/Transform.h"
#include "Scene/Lights.h"
#include "Scene/Screen.h"
#include "Utilities/Logger.h"
#include <imgui.h>

TestBistro::TestBistro() {
    // Initialization if needed
}

void TestBistro::OnEnter() {

    m_Camera = std::make_shared<Camera>(
        glm::vec3(10.0f, 5.0f, 15.0f), // Position: further away for a large scene
        glm::vec3(0.0f, 1.0f, 0.0f),   // Up vector
        -135.0f,                       // Yaw: facing towards the origin
        -20.0f                         // Pitch: looking slightly downward
    );
    m_Camera->SetSpeed(200.0f);
    m_Camera->SetFarPlane(10000.0f);
    m_Scene->SetCamera(m_Camera);

    auto& modelManager = ModelManager::GetInstance();
    auto& materialManager = MaterialManager::GetInstance();
    auto& shaderManager = ShaderManager::GetInstance();

    // Define mesh layout
    MeshLayout objMeshLayout = {
        true,  // Positions
        false,  // Normals
        false, // Tangents
        false, // Bitangents
        {}     // Texture Coordinates
    };

    // Get model
    auto model = modelManager.GetModel("bistro");
    if (!model) {
        Logger::GetLogger()->error("Failed to load model 'bistro'");
        return;
    }

    // Create material
    auto material = std::make_shared<Material>();

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
            "wireframeGL",
            transform
        );
        m_Scene->AddRenderObject(renderObject);
    }

    m_Scene->BuildBatches();
    //m_Scene->SetPostProcessingEffect(PostProcessingEffectType::EdgeDetection);
}

void TestBistro::OnExit() {
    m_Renderer.reset();
    m_Scene->Clear();
}

void TestBistro::OnUpdate(float deltaTime) {
    // Update objects or animations if needed
}


void TestBistro::OnImGuiRender() {
    ImGui::Begin("TestBistro Controls");
    glm::vec3& position = m_Camera->GetPositionRef();
    ImGui::SliderFloat3("Camera Position", &position.x, 0.0f, 2000.0f);
    ImGui::End();
}
