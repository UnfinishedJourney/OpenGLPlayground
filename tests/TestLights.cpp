#include "TestLights.h"
#include "Resources/ModelManager.h"
#include "Resources/MaterialManager.h"
#include "Resources/ShaderManager.h"
#include "Scene/Transform.h"
#include "Scene/Lights.h"
#include "Scene/Screen.h"
#include "Scene/SceneNode.h"
#include "Utilities/Logger.h"
#include "Graphics/Meshes/ModelLoader.h"
#include <imgui.h>

TestLights::TestLights()
    : Test()
{
    // ...
}

void TestLights::OnEnter() {
    auto& materialManager = MaterialManager::GetInstance();
    auto& shaderManager = ShaderManager::GetInstance();

    // Create the gold material
    auto goldMaterial = std::make_shared<Material>();
    goldMaterial->AddParam<glm::vec3>("material.Ka", glm::vec3(0.24725f, 0.1995f, 0.0745f));
    goldMaterial->AddParam<glm::vec3>("material.Kd", glm::vec3(0.75164f, 0.60648f, 0.22648f));
    goldMaterial->AddParam<glm::vec3>("material.Ks", glm::vec3(0.628281f, 0.555802f, 0.366065f));
    goldMaterial->AddParam<float>("material.shininess", 51.2f);

    materialManager.AddMaterial("objMaterial", goldMaterial);

    // Define mesh layout
    MeshLayout objMeshLayout = {
        true,  // Positions
        true,  // Normals
        false, // Tangents
        false, // Bitangents
        {}     // Texture Coordinates
    };

    // Use ModelLoader to load pig model into the scene graph
    ModelLoader modelLoader;
    auto pigNode = modelLoader.LoadModelIntoSceneGraph(
        "pig",           // key or path
        "simplelights",  // defaultShader
        "objMaterial",   // defaultMaterial
        objMeshLayout
    );
    if (!pigNode) {
        Logger::GetLogger()->error("Failed to load pig model node");
        return;
    }

    // Optionally set local transform on pigNode if you want to move it up
    glm::mat4 localTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 0.0f));
    pigNode->SetLocalTransform(localTransform);

    // Attach pigNode to scene root
    m_Scene->GetRootNode()->AddChild(pigNode);

    // Add some lights
    LightData light1 = { glm::vec4(1.5f, 2.0f, 1.5f, 0.0f), glm::vec4(1.0f,1.0f,1.0f,1.0f) };
    m_Scene->AddLight(light1);
    LightData light2 = { glm::vec4(-1.5f,2.0f,-1.5f,0.0f), glm::vec4(1.0f,0.0f,0.0f,1.0f) };
    m_Scene->AddLight(light2);

    // Now the scene is ready
    m_Scene->SetBDebugLights(true);
    m_Scene->SetBGrid(true);
}

void TestLights::OnExit() {
    m_Renderer.reset();
    m_Scene->Clear();
}

void TestLights::OnUpdate(float deltaTime) {
    // Update logic if needed
}

void TestLights::OnImGuiRender() {
    ImGui::Begin("TestLights Controls");
    ImGui::Text("Use this panel for debugging or adjustments");
    ImGui::End();
}