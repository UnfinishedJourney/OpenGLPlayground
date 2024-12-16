//#pragma once
//
//#include "Test.h"
//#include "Graphics/Shaders/ComputeShader.h"
//
//class TestComputeShader : public Test {
//public:
//    TestComputeShader();
//    ~TestComputeShader() override = default;
//
//    void OnEnter() override;
//    void OnExit() override;
//    void OnUpdate(float deltaTime) override;
//    void OnImGuiRender() override;
//
//private:
//    std::unique_ptr<ComputeShader> m_ComputeShader;
//    std::shared_ptr<ITexture> m_ComputedTexture;
//    // OnRender, OnWindowResize, and GetCamera inherited from Test
//};