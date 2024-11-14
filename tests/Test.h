#pragma once

#include <vector>
#include <functional>
#include <string>
#include <memory>
#include "Scene/Scene.h"
#include "Renderer/Renderer.h"

namespace test {

    class Test
    {
    public:
        Test() = default;
        virtual ~Test() = default;

        virtual void OnEnter() {}
        virtual void OnExit() {}
        virtual void OnUpdate(float deltaTime) {}
        virtual void OnRender() {}
        virtual void OnImGuiRender() {}
        virtual void OnWindowResize(int width, int height);

    protected:
        std::unique_ptr<Renderer> m_Renderer;
    };

    class TestMenu : public Test
    {
    public:
        TestMenu(std::shared_ptr<Test>& currentTest);
        void OnImGuiRender() override;

        template<typename T>
        void RegisterTest(const std::string& name)
        {
            m_Tests.push_back({ name, []() { return std::make_shared<T>(); } });
        }

    private:
        std::shared_ptr<Test>& m_CurrentTest;
        std::vector<std::pair<std::string, std::function<std::shared_ptr<Test>()>>> m_Tests;
    };
}
