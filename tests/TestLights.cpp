#include "TestLights.h"
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

    void LightObject::Draw()
    {
        m_MeshBuffer->Bind();
        m_Material->Bind();
        glm::mat4 mvp = FrameData::s_Projection * FrameData::s_View * m_Transform->GetModelMatrix();
        m_Material->GetShader()->SetUniformMat4f("u_MVP", mvp);
        glm::mat4 mv = FrameData::s_View * m_Transform->GetModelMatrix();
        m_Material->GetShader()->SetUniformMat4f("u_ModelView", mv);
        glm::mat3 normalMatrix = glm::mat3(mv);
        m_Material->GetShader()->SetUniformMat3f("u_NormalMatrix", normalMatrix);
        GLCall(glDrawElements(GL_TRIANGLES, m_MeshBuffer->GetNVerts(), GL_UNSIGNED_INT, nullptr));
    }

    TestLights::TestLights()
    {
        m_Scene = std::make_unique<Scene>();
        float x(0.0), y(0.0), z(0.0);
        for (int i = 0; i < 4; i++)
        {
            x = 2.0f * cosf((glm::two_pi<float>() / 4) * i);
            z = 2.0f * sinf((glm::two_pi<float>() / 4) * i);
            m_Scene->AddLight({ glm::vec4(x, 1.2f, z + 1.0f, 1.0f) , glm::vec3(0.8f, 0.8f, 0.8f) });
        }

        m_ResourceManager = std::make_unique<ResourceManager>();
        auto model = m_ResourceManager->GetModel("duck");
        MeshLayout meshLayout = { true, true, false, false, false };
        std::shared_ptr<MeshBuffer> meshBuffer = model->GetMeshBuffer(0, meshLayout);
        std::shared_ptr<Shader> shader = m_ResourceManager->GetShader("lights");
        std::shared_ptr<Material> material = std::make_shared<Material>();
        std::unique_ptr<Transform> transform = std::make_unique<Transform>();

        material->SetShader(shader);
        material->AddParam("Ks", 1.0);
        material->AddParam("Kd", 1.0);
        material->AddParam("Shininess", 1.0);

        auto renderObject = std::make_unique<LightObject>(meshBuffer, material, std::move(transform));

        m_Scene->AddObj(std::move(renderObject));

        m_Scene->AddLightShader(shader);

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
        m_Scene->Render();
    }

    void TestLights::OnImGuiRender()
    {
    }

}