//#include "TestSkyBox.h"
//#include "Renderer/Renderer.h"
//#include "Resources/ResourceManager.h"
//
//namespace test {
//
//    TestSkyBox::TestSkyBox()
//        : m_SkyboxTextureName("pisa"), m_SkyboxShaderName("skyBox")
//    {
//    }
//
//    void TestSkyBox::OnEnter()
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
//        // Ensure that the skybox texture and shader are loaded
//        //without it there is an error, need to check
//        resourceManager.GetCubeMapTexture(m_SkyboxTextureName);
//        resourceManager.GetShader(m_SkyboxShaderName);
//    }
//
//    void TestSkyBox::OnExit()
//    {
//        m_SkyboxMeshBuffer.reset();
//    }
//
//    void TestSkyBox::OnUpdate(float deltaTime)
//    {
//    }
//
//    void TestSkyBox::OnRender()
//    {
//        Renderer::GetInstance().Clear(0.3f, 0.4f, 0.55f, 1.0f);
//        Renderer::GetInstance().RenderSkybox(m_SkyboxMeshBuffer, m_SkyboxTextureName, m_SkyboxShaderName);
//    }
//
//    void TestSkyBox::OnImGuiRender()
//    {
//        // Add ImGui controls if needed
//    }
//
//}