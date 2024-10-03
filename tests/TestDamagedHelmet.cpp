#include "TestDamagedHelmet.h"
#include "Renderer/Renderer.h"
#include "imgui.h"

#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <ext.hpp>
#include <iostream>

namespace test {

    const unsigned int BRDF_W = 256;
    const unsigned int BRDF_H = 256;

    void MovingDamagedHelmet::Render()
    {
        for (auto& meshBuffer : m_MeshBuffers)
        {
            meshBuffer->Bind();
            m_Material->Bind();
            glm::mat4 mvp = FrameData::s_Projection * FrameData::s_View * m_Transform->GetModelMatrix();
            //m_Material->GetShader()->SetUniform("u_MVP", mvp);
            m_Material->GetShader()->SetUniform("u_Model", m_Transform->GetModelMatrix());
            m_Material->GetShader()->SetUniform("u_View", FrameData::s_View);
            m_Material->GetShader()->SetUniform("u_Proj", FrameData::s_Projection);

            m_Material->GetShader()->SetUniform("lightPosition", glm::vec3(2.0, 1.2, 2.0));
            m_Material->GetShader()->SetUniform("lightColor", glm::vec3(1.0, 1.0, 1.0));
            m_Material->GetShader()->SetUniform("cameraPos", glm::vec3(0.0f, 0.0f, 3.0f));
            //glm::mat4 mv = FrameData::s_View * m_Transform->GetModelMatrix();
            //m_Material->GetShader()->SetUniform("u_ModelView", mv);
            //glm::mat3 normalMatrix = glm::mat3(mv);
            //m_Material->GetShader()->SetUniform("u_NormalMatrix", normalMatrix);

            GLCall(glDrawElements(GL_TRIANGLES, meshBuffer->GetNVerts(), GL_UNSIGNED_INT, nullptr));
        }
    }

    TestDamagedHelmet::TestDamagedHelmet()
    {
        m_ComputeShader = s_ResourceManager->GetComputeShader("brdf_compute");

        glGenBuffers(1, &m_DSTBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_DSTBuffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, BRDF_W * BRDF_H * sizeof(glm::vec2), nullptr, GL_DYNAMIC_COPY);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_DSTBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        glGenTextures(1, &m_BRDFLUTTextureID);
        glBindTexture(GL_TEXTURE_2D, m_BRDFLUTTextureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, BRDF_W, BRDF_H, 0, GL_RG, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);


        m_ComputeShader->Bind();

        m_ComputeShader->SetUniform("NUM_SAMPLES", 1024u);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_DSTBuffer);

        GLuint groupX = (BRDF_W + 15) / 16;
        GLuint groupY = (BRDF_H + 15) / 16;

        glDispatchCompute(groupX, groupY, 1);

        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT | GL_UNIFORM_BARRIER_BIT);

        m_ComputeShader->Unbind();

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_DSTBuffer);
        glm::vec2* ptr = (glm::vec2*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, BRDF_W * BRDF_H * sizeof(glm::vec2), GL_MAP_READ_BIT);
        if (ptr)
        {
            glBindTexture(GL_TEXTURE_2D, m_BRDFLUTTextureID);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, BRDF_W, BRDF_H, GL_RG, GL_FLOAT, ptr);
            glBindTexture(GL_TEXTURE_2D, 0);
            glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        }
        else
        {
            std::cerr << "Did not manage to map dst buffer" << std::endl;
        }

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 10);


        auto helmetModel = s_ResourceManager->GetModel("damagedHelmet");
        if (!helmetModel) {
            std::cerr << "Bads model 'damagedHelmet'\n";
            return;
        }

        MeshLayout helmetMeshLayout = {
            true,  // hasPositions
            true,  // hasNormals
            false, // hasBinormals
            false, // hasTangents
            true   // hasUVs
        };

        std::shared_ptr<MeshBuffer> helmetMeshBuffer = helmetModel->GetMeshBuffer(0, helmetMeshLayout);
        if (!helmetMeshBuffer) {
            std::cerr << "Bad MeshBuffer\n";
            return;
        }

        std::shared_ptr<Shader> shader = s_ResourceManager->GetShader("helmet");
        if (!shader) {
            std::cerr << "Bad Shader\n";
            return;
        }

        std::shared_ptr<Texture2D> albedoTexture = helmetModel->GetTexture(0, TextureType::Albedo);
        std::shared_ptr<Texture2D> normalTexture = helmetModel->GetTexture(0, TextureType::Normal);
        std::shared_ptr<Texture2D> occlusionTexture = helmetModel->GetTexture(0, TextureType::Occlusion);
        std::shared_ptr<Texture2D> roughmetTexture = helmetModel->GetTexture(0, TextureType::RoughnessMetallic);
        std::shared_ptr<Texture2D> emissiveTexture = helmetModel->GetTexture(0, TextureType::Emissive);

        std::shared_ptr<Material> material = std::make_shared<Material>();
        material->SetShader(shader);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_BRDFLUTTextureID);
        material->AddTexture(albedoTexture, 1); 
        material->AddTexture(normalTexture, 2);
        material->AddTexture(roughmetTexture, 3);
        material->AddTexture(occlusionTexture, 4);
        material->AddTexture(emissiveTexture, 5);
        std::unique_ptr<Transform> transform = std::make_unique<Transform>();
        transform->SetRotation(glm::vec3(-1.57f, 0.0f, 0.0f)); 


        m_HeHelmetRenderObject = std::make_unique<MovingDamagedHelmet>(helmetMeshBuffer, material, std::move(transform));

        GLCall(glEnable(GL_DEPTH_TEST));
    }

    TestDamagedHelmet::~TestDamagedHelmet()
    {
    }

    void TestDamagedHelmet::OnUpdate(float deltaTime)
    {
        if (m_HeHelmetRenderObject) {
            m_HeHelmetRenderObject->Update(deltaTime);
        }
    }

    void TestDamagedHelmet::OnRender()
    {
        GLCall(glClearColor(0.1f, 0.1f, 0.1f, 1.0f));
        GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        if (m_HeHelmetRenderObject) {
            m_HeHelmetRenderObject->Render();
        }
    }

    void TestDamagedHelmet::OnImGuiRender()
    {
        ImGui::Begin("Damaged Helmet Controls");
        if (ImGui::Button("Reload All Shaders")) {
            s_ResourceManager->ReloadAllShaders();
        }
        ImGui::End();
    }

}