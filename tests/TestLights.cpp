//#include "TestLights.h"
//#include "Renderer/Renderer.h"
//#include "imgui.h"
//
//#include <GLFW/glfw3.h>
//
//#include <glm/glm.hpp>
//#include <glm/ext.hpp>
//#include <assimp/scene.h>
//#include <assimp/postprocess.h>
//#include <assimp/cimport.h>
//#include <assimp/version.h>
//
//#include <stdio.h>
//#include <stdlib.h>
//
//#include <vector>
//#include <numeric>
//
//namespace test {
//
//    TestLights::TestLights(std::shared_ptr<Renderer>& renderer)
//        : Test(renderer)
//    {
//        //float x(0.0), y(0.0), z(0.0);
//        //for (int i = 0; i < 4; i++)
//        //{
//        //    x = 2.0f * cosf((glm::two_pi<float>() / 4) * i);
//        //    z = 2.0f * sinf((glm::two_pi<float>() / 4) * i);
//        //    m_Scene->AddLight({ glm::vec4(x, 1.2f, z + 1.0f, 1.0f) , glm::vec3(0.8f, 0.8f, 0.8f) });
//        //}
//
//        MeshLayout pigMeshLayout = {
//        true,
//        true,
//        false,
//        false,
//        {}
//        };
//
//        auto meshInfos = m_Renderer->m_ResourceManager->GetModelMeshInfos("pig");
//        std::shared_ptr<Shader> shader = m_Renderer->m_ResourceManager->GetShader("simplelights");
//
//        std::shared_ptr<Material> material = std::make_shared<Material>();
//        std::shared_ptr<Transform> transform = std::make_shared<Transform>();
//        material->AddParam<glm::vec3>("material.Ka", glm::vec3(1.0, 0.0, 0.0));
//        material->AddParam<glm::vec3>("material.Kd", glm::vec3(1.0, 0.0, 0.0));
//        material->AddParam<glm::vec3>("material.Ks", glm::vec3(1.0, 0.0, 0.0));
//        material->AddParam<float>("material.shininess", 1.0);
//
//        m_Renderer->m_ResourceManager->AddMaterial("pigMat", material);
//
//        for (auto&[meshTextures, mesh] : meshInfos)
//        {
//            auto ro = std::make_shared<LightObject>(mesh, pigMeshLayout, "pigMat", "simplelights", transform);
//            m_Renderer->AddRenderObject(ro);
//        }
//
//        //for (auto& meshComponent : meshComponents)
//        //{
//        //    std::shared_ptr<Transform> transform = std::make_shared<Transform>();
//        //    auto lightObject = std::make_shared<LightObject>(meshComponent, "pigMat", "simplelights", transform);
//        //    m_Pig.push_back(lightObject);
//
//        //    // Add RenderObject to Renderer for batching
//        //    m_Renderer->AddRenderObject(lightObject);
//        //}
//
//        GLCall(glEnable(GL_BLEND));
//        GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
//
//
//    }
//
//    void TestLights::OnUpdate(float deltaTime)
//    {
//    }
//
//    void TestLights::OnRender()
//    {
//        m_Renderer->Clear();
//        m_Renderer->RenderScene();
//        //for (auto pigPart: m_Pig)
//            //m_Renderer->Render(pigPart);
//    }
//
//    void TestLights::OnImGuiRender()
//    {
//    }
//
//}


#include "TestLights.h"
#include "Renderer/Renderer.h"
#include "Resources/ResourceManager.h"
#include "Graphics/Meshes/Mesh.h"
#include "Scene/Transform.h"
#include <GLFW/glfw3.h>

namespace test {

    TestLights::TestLights()
    {
    }

    void TestLights::OnEnter()
    {
        auto& resourceManager = ResourceManager::GetInstance();

        MeshLayout pigMeshLayout = {
            true, // Positions
            true, // Normals
            false, // Texture Coordinates
            false, // Tangents and Bitangents
            {}
        };

        auto meshInfos = resourceManager.GetModelMeshInfos("pig");
        auto shader = resourceManager.GetShader("simplelights");

        auto material = std::make_shared<Material>();
        material->AddParam<glm::vec3>("material.Ka", glm::vec3(1.0, 0.0, 0.0));
        material->AddParam<glm::vec3>("material.Kd", glm::vec3(1.0, 0.0, 0.0));
        material->AddParam<glm::vec3>("material.Ks", glm::vec3(1.0, 0.0, 0.0));
        material->AddParam<float>("material.shininess", 1.0);

        resourceManager.AddMaterial("pigMaterial", material);

        auto transform = std::make_shared<Transform>();

        auto& renderer = Renderer::GetInstance();

        for (auto& [meshTextures, mesh] : meshInfos)
        {
            auto ro = std::make_shared<RenderObject>(mesh, pigMeshLayout, "pigMaterial", "simplelights", transform);
            renderer.AddRenderObject(ro);
        }
           
        GLCall(glEnable(GL_BLEND));
        GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    }

    void TestLights::OnExit()
    {
        m_Cube.reset();
    }

    void TestLights::OnUpdate(float deltaTime)
    {
        //if (m_Cube)
        //{
        //    m_Cube->GetTransform()->AddRotation(glm::vec3(0.0, deltaTime, 0.0));
        //}
    }

    void TestLights::OnRender()
    {
        Renderer::GetInstance().Clear(0.3f, 0.4f, 0.55f, 1.0f);
        Renderer::GetInstance().RenderScene();
    }

    void TestLights::OnImGuiRender()
    {
        // Add ImGui controls if needed
    }

}