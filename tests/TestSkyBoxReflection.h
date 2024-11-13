//#pragma once
//
//#include "Test.h"
//#include "Graphics/Buffers/MeshBuffer.h"
//#include <memory>
//#include <string>
//
//namespace test {
//
//    class TestSkyBoxReflection : public Test
//    {
//    public:
//        TestSkyBoxReflection();
//        ~TestSkyBoxReflection() override = default;
//
//        void OnEnter() override;
//        void OnExit() override;
//        void OnUpdate(float deltaTime) override;
//        void OnRender() override;
//        void OnImGuiRender() override;
//
//    private:
//
//        std::shared_ptr<MeshBuffer> m_SkyboxMeshBuffer;
//        std::string m_SkyboxTextureName;
//        std::string m_SkyboxShaderName;
//    };
//
//}