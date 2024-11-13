//#include "TestSkyBoxReflection.h"
//#include "Renderer/Renderer.h"
//#include "Resources/ResourceManager.h"
//#include "Scene/Transform.h"
//
//namespace test {
//
//    TestSkyBoxReflection::TestSkyBoxReflection()
//        : m_SkyboxTextureName("pisa"), m_SkyboxShaderName("skyBox")
//    {
//    }
//
//    void TestSkyBoxReflection::OnEnter()
//    {
//        auto& resourceManager = ResourceManager::GetInstance();
//
//        MeshLayout skyBoxMeshLayout = {
//            true, // Positions
//            false, // Normals
//            false, // Texture Coordinates
//            false, // Tangents and Bitangents
//            {}
//        };
//
//        m_SkyboxMeshBuffer = resourceManager.GetMeshBuffer("cube", skyBoxMeshLayout);
//
//        auto sphereMesh = resourceManager.GetMesh("sphere");
//        MeshLayout sphereMeshLayout = {
//            true, // Positions
//            true, // Normals
//            false, // Texture Coordinates
//            false, // Tangents and Bitangents
//            {}
//        };
//
//        auto shader = resourceManager.GetShader("reflectSkybox");
//        auto texture = resourceManager.GetCubeMapTexture(m_SkyboxTextureName);
//
//        auto material = std::make_shared<Material>();
//        material->AddTexture(texture);
//        resourceManager.AddMaterial("sphereMaterial", material);
//
//        auto transform = std::make_shared<Transform>();
//
//        auto sphere = std::make_shared<RenderObject>(sphereMesh, sphereMeshLayout, "sphereMaterial", "reflectSkybox", transform);
//        Renderer::GetInstance().AddRenderObject(sphere);
//        resourceManager.GetShader(m_SkyboxShaderName);
//    }
//
//    void TestSkyBoxReflection::OnExit()
//    {
//        Renderer::GetInstance().ClearRenderObjects();
//    }
//
//    void TestSkyBoxReflection::OnUpdate(float deltaTime)
//    {
//        // Update camera position or object transformations if needed
//        // For example, rotating the object:
//        // m_ObjectTransform->AddRotation(glm::vec3(0.0f, deltaTime, 0.0f));
//    }
//
//    void TestSkyBoxReflection::OnRender()
//    {
//        Renderer::GetInstance().Clear(0.3f, 0.4f, 0.55f, 1.0f);
//        Renderer::GetInstance().RenderSkybox(m_SkyboxMeshBuffer, m_SkyboxTextureName, m_SkyboxShaderName);
//        Renderer::GetInstance().RenderScene();
//    }
//
//    void TestSkyBoxReflection::OnImGuiRender()
//    {
//        // Add ImGui controls if needed
//        // For example, allow the user to move the camera or rotate the object
//    }
//
//}