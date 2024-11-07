#pragma once

#include "Test.h"
#include "Renderer/RenderObject.h"
#include <memory>

namespace test {

    class TestLights : public Test
    {
    public:
        TestLights();
        ~TestLights() override = default;

        void OnEnter() override;
        void OnExit() override;
        void OnUpdate(float deltaTime) override;
        void OnRender() override;
        void OnImGuiRender() override;

    private:
        std::shared_ptr<RenderObject> m_Cube;
    };

}