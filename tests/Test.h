#pragma once

#include <memory>
#include "Scene/Camera.h"
#include "Renderer/Renderer.h"
#include "Scene/Scene.h"
#include "Scene/Screen.h"

class Test {
public:
    Test() 
    {
        m_Renderer = std::make_unique<Renderer>();
        m_Scene = std::make_shared<Scene>();

        int width = Screen::s_Width;
        int height = Screen::s_Height;
        m_Renderer->Initialize(width, height);

        auto camera = std::make_shared<Camera>();
        m_Scene->SetCamera(camera);
    }

    virtual ~Test() = default;

    virtual void OnEnter() {
    }

    virtual void OnExit() {
        // Default implementation
        m_Renderer.reset();
        if (m_Scene) {
            m_Scene->Clear();
        }
    }

    virtual void OnUpdate(float deltaTime) {}
    virtual void OnRender() {
        if (m_Renderer && m_Scene) {
            m_Renderer->RenderScene(m_Scene);
        }
    }
    virtual void OnImGuiRender() {}
    virtual void OnWindowResize(int width, int height) {
        if (m_Renderer) {
            m_Renderer->OnWindowResize(width, height);
        }
    }
    virtual std::shared_ptr<Camera> GetCamera() const {
        if (m_Scene) {
            return m_Scene->GetCamera();
        }
        return nullptr;
    }

protected:
    std::unique_ptr<Renderer> m_Renderer;
    std::shared_ptr<Scene> m_Scene;
};