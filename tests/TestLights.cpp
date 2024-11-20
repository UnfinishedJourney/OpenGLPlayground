//#include "TestLights.h"
//#include "Renderer/Renderer.h"
//#include "Resources/ResourceManager.h"
//#include "Graphics/Meshes/Mesh.h"
//#include "Scene/Transform.h"
//#include "Scene/Screen.h"
//#include "Scene/Lights.h"
//
//namespace test {
//
//    TestLights::TestLights()
//    {
//    }
//
//    void TestLights::OnEnter()
//    {
//
//        auto& modelManager = ModelManager::GetInstance();
//        auto& materialManager = MaterialManager::GetInstance();
//        auto& shaderManager = ShaderManager::GetInstance();
//        auto& meshManager = MeshManager::GetInstance();
//
//        // Define mesh layout
//        MeshLayout pigMeshLayout = {
//            true,  // Positions
//            true,  // Normals
//            false, // Texture Coordinates
//            false, // Tangents and Bitangents
//            {}
//        };
//
//        // Get model
//        auto model = modelManager.GetModel("pig");
//        if (!model) {
//            Logger::GetLogger()->error("Failed to load model 'pig'");
//            return;
//        }
//
//        // Create material
//        auto material = std::make_shared<Material>();
//
//        // Gold material properties
//        material->AddParam<glm::vec3>("material.Ka", glm::vec3(0.24725f, 0.1995f, 0.0745f));
//        material->AddParam<glm::vec3>("material.Kd", glm::vec3(0.75164f, 0.60648f, 0.22648f));
//        material->AddParam<glm::vec3>("material.Ks", glm::vec3(0.628281f, 0.555802f, 0.366065f));
//        material->AddParam<float>("material.shininess", 51.2f);
//
//        materialManager.AddMaterial("pigMaterial", material);
//
//        auto transform = std::make_shared<Transform>();
//        transform->SetPosition(glm::vec3(0.0, 0.5, 0.0));
//
//        // Get mesh buffers from the model
//        auto meshBuffers = model->GetMeshBuffers(pigMeshLayout);
//        const auto& meshInfos = model->GetMeshesInfo();
//
//        // Add render objects to the scene
//        for (size_t i = 0; i < meshBuffers.size(); ++i) {
//            const auto& meshBuffer = meshBuffers[i];
//            const auto& meshInfo = meshInfos[i];
//
//            auto renderObject = std::make_shared<RenderObject>(
//                meshBuffer,
//                pigMeshLayout,
//                "pigMaterial",
//                "simplelights",
//                transform
//            );
//            m_Scene->AddRenderObject(renderObject);
//        }
//
//        // Build batches
//        m_Scene->BuildBatches();
//        m_Scene->SetBDebugLights(true);
//        m_Scene->SetPostProcessingEffect(PostProcessingEffectType::EdgeDetection);
//    }
//
//    void TestLights::OnExit()
//    {
//        m_Renderer.reset(); 
//        m_Scene->Clear();
//    }
//
//    void TestLights::OnUpdate(float deltaTime)
//    {
//        // Update objects or animations if needed
//    }
//
//    void TestLights::OnRender()
//    {
//        if (m_Renderer && m_Scene)
//        {
//            m_Renderer->RenderScene(m_Scene);
//        }
//    }
//
//    void TestLights::OnImGuiRender()
//    {
//        // Add ImGui controls if needed
//    }
//
//}