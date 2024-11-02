#include "TestLights.h"
#include "Renderer/Renderer.h"
#include "imgui.h"

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include <assimp/version.h>

#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <numeric>

namespace test {

    TestLights::TestLights(std::shared_ptr<Renderer>& renderer)
        : Test(renderer)
    {
        //float x(0.0), y(0.0), z(0.0);
        //for (int i = 0; i < 4; i++)
        //{
        //    x = 2.0f * cosf((glm::two_pi<float>() / 4) * i);
        //    z = 2.0f * sinf((glm::two_pi<float>() / 4) * i);
        //    m_Scene->AddLight({ glm::vec4(x, 1.2f, z + 1.0f, 1.0f) , glm::vec3(0.8f, 0.8f, 0.8f) });
        //}

        MeshLayout pigMeshLayout = {
        true,
        false,
        false,
        false,
        {}
        };

        auto meshComponents = m_Renderer->m_ResourceManager->GetModelMeshBuffers("pig", pigMeshLayout);
        std::shared_ptr<Shader> shader = m_Renderer->m_ResourceManager->GetShader("simplelights");

        std::shared_ptr<Material> material = std::make_shared<Material>();
        std::shared_ptr<Transform> transform = std::make_shared<Transform>();
        material->AddParam<glm::vec3>("material.Ka", glm::vec3(1.0, 0.0, 0.0));
        material->AddParam<glm::vec3>("material.Kd", glm::vec3(1.0, 0.0, 0.0));
        material->AddParam<glm::vec3>("material.Ks", glm::vec3(1.0, 0.0, 0.0));
        material->AddParam<float>("material.shininess", 1.0);

        m_Renderer->m_ResourceManager->AddMaterial("pigMat", material);

        for (auto meshComponent : meshComponents)
        {
            m_Pig.push_back(std::make_unique<LightObject>(meshComponent, "pigMat"_mt, "simplelights"_sh, transform));
        }
        GLCall(glEnable(GL_BLEND));
        GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));


    }

    void TestLights::OnUpdate(float deltaTime)
    {
    }

    void TestLights::OnRender()
    {
        GLCall(glClearColor(0.3f, 0.4f, 0.55f, 1.0f));
        GLCall(glClear(GL_COLOR_BUFFER_BIT));
        for (auto pigPart: m_Pig)
            m_Renderer->Render(pigPart);
    }

    void TestLights::OnImGuiRender()
    {
    }

}