#pragma once

#include "Test.h"
#include "Scene/Scene.h"
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

        std::shared_ptr<Scene> GetScene() const override { return m_Scene; }

    private:
        std::shared_ptr<Scene> m_Scene;
    };

}