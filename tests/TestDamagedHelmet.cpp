//#include "TestDamagedHelmet.h"
//#include "Renderer/Renderer.h"
//#include "imgui.h"
//
//#include <GLFW/glfw3.h>
//#include <glm.hpp>
//#include <ext.hpp>
//#include <iostream>
//
//namespace test {
//
//    const unsigned int BRDF_W = 256;
//    const unsigned int BRDF_H = 256;
//
//    void MovingDamagedHelmet::Render()
//    {
//        for (auto& meshBuffer : m_MeshBuffers)
//        {
//            meshBuffer->Bind();
//            m_Material->Bind();
//            glm::mat4 mvp = FrameData::s_Projection * FrameData::s_View * m_Transform->GetModelMatrix();
//            //m_Material->GetShader()->SetUniform("u_MVP", mvp);
//            m_Material->GetShader()->SetUniform("u_Model", m_Transform->GetModelMatrix());
//            m_Material->GetShader()->SetUniform("u_View", FrameData::s_View);
//            m_Material->GetShader()->SetUniform("u_Proj", FrameData::s_Projection);
//
//            m_Material->GetShader()->SetUniform("lightPosition", glm::vec3(2.0, 1.2, 2.0));
//            m_Material->GetShader()->SetUniform("lightColor", glm::vec3(1.0, 1.0, 1.0));
//            m_Material->GetShader()->SetUniform("cameraPos", glm::vec3(0.0f, 0.0f, 3.0f));
//            //glm::mat4 mv = FrameData::s_View * m_Transform->GetModelMatrix();
//            //m_Material->GetShader()->SetUniform("u_ModelView", mv);
//            //glm::mat3 normalMatrix = glm::mat3(mv);
//            //m_Material->GetShader()->SetUniform("u_NormalMatrix", normalMatrix);
//
//            GLCall(glDrawElements(GL_TRIANGLES, meshBuffer->GetNVerts(), GL_UNSIGNED_INT, nullptr));
//        }
//    }
//
//    void MovingDamagedHelmet::Unbind()
//    {
//        for (auto& meshBuffer : m_MeshBuffers)
//        {
//            meshBuffer->Unbind();
//        }
//    }
//
//    TestDamagedHelmet::TestDamagedHelmet()
//    {
//        m_ComputeShader = s_ResourceManager->GetComputeShader("brdf_compute");
//
//        glGenBuffers(1, &m_DSTBuffer);
//        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_DSTBuffer);
//        glBufferData(GL_SHADER_STORAGE_BUFFER, BRDF_W * BRDF_H * sizeof(glm::vec2), nullptr, GL_DYNAMIC_COPY);
//        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_DSTBuffer);
//        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
//
//        glGenTextures(1, &m_BRDFLUTTextureID);
//        glBindTexture(GL_TEXTURE_2D, m_BRDFLUTTextureID);
//        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, BRDF_W, BRDF_H, 0, GL_RG, GL_FLOAT, nullptr);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//        glBindTexture(GL_TEXTURE_2D, 0);
//
//
//        m_ComputeShader->Bind();
//
//        m_ComputeShader->SetUniform("NUM_SAMPLES", 1024u);
//
//        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_DSTBuffer);
//
//        GLuint groupX = (BRDF_W + 15) / 16;
//        GLuint groupY = (BRDF_H + 15) / 16;
//
//        glDispatchCompute(groupX, groupY, 1);
//
//        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT | GL_UNIFORM_BARRIER_BIT);
//
//        m_ComputeShader->Unbind();
//
//        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_DSTBuffer);
//        glm::vec2* ptr = (glm::vec2*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, BRDF_W * BRDF_H * sizeof(glm::vec2), GL_MAP_READ_BIT);
//        if (ptr)
//        {
//            glBindTexture(GL_TEXTURE_2D, m_BRDFLUTTextureID);
//            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, BRDF_W, BRDF_H, GL_RG, GL_FLOAT, ptr);
//            glBindTexture(GL_TEXTURE_2D, 0);
//            glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
//        }
//        else
//        {
//            std::cerr << "Did not manage to map dst buffer" << std::endl;
//        }
//
//        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 10);
//
//
//        auto helmetModel = s_ResourceManager->GetModel("damagedHelmet");
//        if (!helmetModel) {
//            std::cerr << "Bads model 'damagedHelmet'\n";
//            return;
//        }
//
//        MeshLayout helmetMeshLayout = {
//            true,  // hasPositions
//            true,  // hasNormals
//            false, // hasBinormals
//            true, // hasTangents
//            {TextureType::Albedo}   // hasUVs
//        };
//
//        std::shared_ptr<MeshBuffer> helmetMeshBuffer = helmetModel->GetMeshBuffer(0, helmetMeshLayout);
//        if (!helmetMeshBuffer) {
//            std::cerr << "Bad MeshBuffer\n";
//            return;
//        }
//
//        std::shared_ptr<Shader> shader = s_ResourceManager->GetShader("helmet");
//        if (!shader) {
//            std::cerr << "Bad Shader\n";
//            return;
//        }
//
//        std::shared_ptr<Texture2D> albedoTexture = helmetModel->GetTexture(0, TextureType::Albedo);
//        std::shared_ptr<Texture2D> normalTexture = helmetModel->GetTexture(0, TextureType::Normal);
//        std::shared_ptr<Texture2D> occlusionTexture = helmetModel->GetTexture(0, TextureType::Occlusion);
//        std::shared_ptr<Texture2D> roughmetTexture = helmetModel->GetTexture(0, TextureType::RoughnessMetallic);
//        std::shared_ptr<Texture2D> emissiveTexture = helmetModel->GetTexture(0, TextureType::Emissive);
//
//        std::shared_ptr<Material> material = std::make_shared<Material>();
//        material->SetShader(shader);
//        glActiveTexture(GL_TEXTURE0);
//        glBindTexture(GL_TEXTURE_2D, m_BRDFLUTTextureID);
//        material->AddTexture(albedoTexture, 1); 
//        material->AddTexture(normalTexture, 2);
//        material->AddTexture(roughmetTexture, 3);
//        material->AddTexture(occlusionTexture, 4);
//        material->AddTexture(emissiveTexture, 5);
//        std::unique_ptr<Transform> transform = std::make_unique<Transform>();
//        transform->SetRotation(glm::vec3(-1.57f, 0.0f, 0.0f)); 
//
//
//        m_HeHelmetRenderObject = std::make_unique<MovingDamagedHelmet>(helmetMeshBuffer, material, std::move(transform));
//
//        GLCall(glEnable(GL_DEPTH_TEST));
//    }
//
//    TestDamagedHelmet::~TestDamagedHelmet()
//    {
//    }
//
//    void TestDamagedHelmet::OnUpdate(float deltaTime)
//    {
//        if (m_HeHelmetRenderObject) {
//            m_HeHelmetRenderObject->Update(deltaTime);
//        }
//    }
//
//    void TestDamagedHelmet::OnRender()
//    {
//        GLCall(glClearColor(0.8f, 0.8f, 0.8f, 1.0f));
//        GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
//
//        if (m_HeHelmetRenderObject) {
//            m_HeHelmetRenderObject->Render();
//        }
//    }
//
//    void TestDamagedHelmet::OnImGuiRender()
//    {
//        ImGui::Begin("Damaged Helmet Controls");
//        if (ImGui::Button("Reload All Shaders")) {
//            m_HeHelmetRenderObject->Unbind();
//            s_ResourceManager->ReloadAllShaders();
//        }
//        ImGui::End();
//    }
//
//}


#include "TestDamagedHelmet.h"
#include "Resources/ModelManager.h"
#include "Resources/MaterialManager.h"
#include "Resources/ShaderManager.h"
#include "Scene/Transform.h"
#include "Scene/Lights.h"
#include "Scene/Screen.h"
#include "Utilities/Logger.h"
#include <imgui.h>

TestDamagedHelmet::TestDamagedHelmet() {
    // Initialization if needed
}

void TestDamagedHelmet::OnEnter() {

    auto& modelManager = ModelManager::GetInstance();
    auto& materialManager = MaterialManager::GetInstance();
    auto& shaderManager = ShaderManager::GetInstance();

    // Define mesh layout
    MeshLayout objMeshLayout = {
        true,  // Positions
        true,  // Normals
        false, // Tangents
        false, // Bitangents
        {}     // Texture Coordinates
    };

    // Get model
    auto model = modelManager.GetModel("helmet");
    if (!model) {
        Logger::GetLogger()->error("Failed to load model 'helmet'");
        return;
    }

    // Create material
    auto material = std::make_shared<Material>();

    // Gold material properties
    material->AddParam<glm::vec3>("material.Ka", glm::vec3(0.24725f, 0.1995f, 0.0745f));
    material->AddParam<glm::vec3>("material.Kd", glm::vec3(0.75164f, 0.60648f, 0.22648f));
    material->AddParam<glm::vec3>("material.Ks", glm::vec3(0.628281f, 0.555802f, 0.366065f));
    material->AddParam<float>("material.shininess", 51.2f);

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
            "simplelights",
            transform
        );
        m_Scene->AddRenderObject(renderObject);
    }

    // Add light to the scene
    LightData light = { glm::vec4(1.5f, 2.0f, 1.5f, 0.0f) , glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) };
    m_Scene->AddLight(light);

    m_Scene->BuildBatches();
    m_Scene->SetBDebugLights(true);
    m_Scene->SetBGrid(true);
}

void TestDamagedHelmet::OnExit() {
    m_Renderer.reset();
    m_Scene->Clear();
}

void TestDamagedHelmet::OnUpdate(float deltaTime) {
    // Update objects or animations if needed
}


void TestDamagedHelmet::OnImGuiRender() {
    ImGui::Begin("TestDamagedHelmet Controls");
    ImGui::End();
}
