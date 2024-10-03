#pragma once

#include "Test.h"
#include "Renderer/RenderObject.h"
#include "Graphics/Meshes/Model.h"
#include "Resources/ResourceManager.h"

#include <memory>

namespace test {

    class MovingDamagedHelmet : public RenderObject {
    public:
        MovingDamagedHelmet(std::shared_ptr<MeshBuffer> meshComp, std::shared_ptr<Material> mat, std::unique_ptr<Transform> transform)
            : RenderObject(meshComp, mat, std::move(transform))
        {}

        void Update(float deltaTime) override {
            m_Transform->AddRotation(glm::vec3(0.0f, deltaTime * 0.5f, 0.0f));
        }

        virtual void Render() override;
    };

    class TestDamagedHelmet : public Test
    {
    public:
        TestDamagedHelmet();
        ~TestDamagedHelmet();

        void OnUpdate(float deltaTime) override;
        void OnRender() override;
        void OnImGuiRender() override;
    private:
        std::unique_ptr<MovingDamagedHelmet> m_HeHelmetRenderObject;
        std::shared_ptr<ComputeShader> m_ComputeShader;

        GLuint m_DSTBuffer;

        GLuint m_BRDFLUTTextureID;
    };

}