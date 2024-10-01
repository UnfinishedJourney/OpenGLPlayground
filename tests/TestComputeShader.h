#pragma once

#include "Test.h"
#include "Resources/ResourceManager.h"

#include <memory>

namespace test {

    class TestComputeShader : public Test
    {
    public:
        TestComputeShader();
        ~TestComputeShader();

        void OnUpdate(float deltaTime) override;
        void OnRender() override;
        void OnImGuiRender() override;
    private:
        //std::unique_ptr<ResourceManager> m_ResourceManager;
        std::shared_ptr<ComputeShader> m_ComputeShader;

        GLuint m_DSTBuffer;

        GLuint m_BRDFLUTTextureID;

        std::shared_ptr<Shader> m_QuadShader;

        GLuint m_QuadVAO, m_QuadVBO;
    };
}