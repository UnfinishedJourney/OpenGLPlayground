#include "TestComputeShader.h"
#include "Renderer/Renderer.h"
#include "imgui.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

namespace test {

    const unsigned int BRDF_W = 256;
    const unsigned int BRDF_H = 256;

    TestComputeShader::TestComputeShader()
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

        m_QuadShader = s_ResourceManager->GetShader("quad");

        float quadVertices[] = {
            -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
             1.0f, -1.0f, 0.0f,  1.0f, 0.0f,

            -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
             1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
             1.0f,  1.0f, 0.0f,  1.0f, 1.0f
        };

        glGenVertexArrays(1, &m_QuadVAO);
        glGenBuffers(1, &m_QuadVBO);

        glBindVertexArray(m_QuadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glBindVertexArray(0);

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
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    TestComputeShader::~TestComputeShader()
    {
        glDeleteBuffers(1, &m_DSTBuffer);
        glDeleteVertexArrays(1, &m_QuadVAO);
        glDeleteBuffers(1, &m_QuadVBO);
        glDeleteTextures(1, &m_BRDFLUTTextureID);
    }

    void TestComputeShader::OnUpdate(float deltaTime)
    {
    }

    void TestComputeShader::OnRender()
    {

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_QuadShader->Bind();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_BRDFLUTTextureID);
        glBindVertexArray(m_QuadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        m_QuadShader->Unbind();
    }

    void TestComputeShader::OnImGuiRender()
    {
        ImGui::Begin("BRDF LUT Visualization");
        if (m_BRDFLUTTextureID)
        {
            ImGui::Image((void*)(intptr_t)m_BRDFLUTTextureID, ImVec2(256, 256), ImVec2(0, 1), ImVec2(1, 0));
        }
        ImGui::End();
    }

}