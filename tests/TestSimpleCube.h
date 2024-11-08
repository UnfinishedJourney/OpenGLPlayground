#pragma once

#include "Test.h"
#include "Renderer/RenderObject.h"
#include <memory>

namespace test {

    class TestSimpleCube : public Test
    {
    public:
        TestSimpleCube();
        ~TestSimpleCube() override = default;

        void OnEnter() override;
        void OnExit() override;
        void OnUpdate(float deltaTime) override;
        void OnRender() override;
        void OnImGuiRender() override;

    private:
    };

}