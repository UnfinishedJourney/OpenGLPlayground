//#pragma once
//
//#include "Graphics/Buffers/MeshBuffer.h"
//#include "Test.h"
//#include <memory>
//
//namespace test {
//
//    class TestSkyBox : public Test
//    {
//    public:
//        TestSkyBox();
//        ~TestSkyBox() override = default;
//
//        void OnEnter() override;
//        void OnExit() override;
//        void OnUpdate(float deltaTime) override;
//        void OnRender() override;
//        void OnImGuiRender() override;
//
//    private:
//        std::shared_ptr<MeshBuffer> m_SkyboxMeshBuffer;
//        std::string m_SkyboxTextureName;
//        std::string m_SkyboxShaderName;
//    };
//
//}