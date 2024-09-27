#include "TestPBR.h"
#include "Renderer.h"
#include "imgui.h"

#include <GLFW/glfw3.h>

#include <glm.hpp>
#include <ext.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include <assimp/version.h>

#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <numeric>

namespace test {

    void PBRObject::Render()
    {
        for (auto& meshBuffer : m_MeshBuffers)
        {
            meshBuffer->Bind();
            m_Material->Bind();
            glm::mat4 mvp = FrameData::s_Projection * FrameData::s_View * m_Transform->GetModelMatrix();
            m_Material->GetShader()->SetUniform("u_MVP", mvp);
            glm::mat4 mv = FrameData::s_View * m_Transform->GetModelMatrix();
            m_Material->GetShader()->SetUniform("u_ModelView", mv);
            glm::mat3 normalMatrix = glm::mat3(mv);
            m_Material->GetShader()->SetUniform("u_NormalMatrix", normalMatrix);
            GLCall(glDrawElements(GL_TRIANGLES, meshBuffer->GetNVerts(), GL_UNSIGNED_INT, nullptr));
        }
    }

    TestPBR::TestPBR()
    {
        m_Scene = std::make_unique<Scene>();
        float x(0.0), y(0.0), z(0.0);
        for (int i = 0; i < 4; i++)
        {
            x = 2.0f * cosf((glm::two_pi<float>() / 4) * i);
            z = 2.0f * sinf((glm::two_pi<float>() / 4) * i);
            m_Scene->AddLight({ glm::vec4(x, 1.2f, z + 1.0f, 1.0f) , glm::vec3(0.8f, 0.8f, 0.8f) });
        }

        auto model = s_ResourceManager->GetModel("pig");
        MeshLayout meshLayout = { true, true, false, false, false };
        std::shared_ptr<Shader> shader = s_ResourceManager->GetShader("pbr");
        std::shared_ptr<Material> material = std::make_shared<Material>();
        std::unique_ptr<Transform> transform = std::make_unique<Transform>();
        transform->SetRotation(glm::vec3(-1.57, 0.0, 0.0));

        material->SetShader(shader);
        material->AddParam<float>("Ks", 1.0);
        material->AddParam<float>("Kd", 1.0);
        material->AddParam<float>("Shininess", 1.0);

        auto renderObject = std::make_unique<PBRObject>(model->GetMeshBuffers(meshLayout), material, std::move(transform));

        m_Scene->AddObj(std::move(renderObject));

        m_Scene->AddLightShader(shader);

        GLCall(glEnable(GL_BLEND));
        GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    }

    void TestPBR::OnUpdate(float deltaTime)
    {
    }

    void TestPBR::OnRender()
    {
        GLCall(glClearColor(0.3f, 0.4f, 0.55f, 1.0f));
        GLCall(glClear(GL_COLOR_BUFFER_BIT));
        m_Scene->Render();
    }

    void TestPBR::OnImGuiRender()
    {
    }

}