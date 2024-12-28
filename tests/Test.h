#pragma once

#include <memory>
#include "Scene/Camera.h"
#include "Renderer/Renderer.h"
#include "Scene/Scene.h"
#include "Scene/Screen.h"

/**
 * @brief Base class representing a "Test" (a self-contained demo/scene).
 */
class Test
{
public:
    Test()
    {
        m_Renderer = std::make_unique<Renderer>();
        m_Scene = std::make_shared<Scene>();

        // Initialize renderer with current screen size
        m_Renderer->Initialize(Screen::s_Width, Screen::s_Height);

        // Attach a default camera
        auto camera = std::make_shared<Camera>();
        m_Scene->SetCamera(camera);
    }

    virtual ~Test() = default;

    virtual void OnEnter() {}
    virtual void OnExit()
    {
        // Default: clean up
        m_Renderer.reset();
        if (m_Scene) {
            m_Scene->Clear();
        }
    }

    virtual void OnUpdate(float /*deltaTime*/) {}
    virtual void OnRender()
    {
        if (m_Renderer && m_Scene) {
            m_Renderer->RenderScene(m_Scene);
        }
    }
    virtual void OnImGuiRender() {}
    virtual void OnWindowResize(int width, int height)
    {
        if (m_Renderer) {
            m_Renderer->OnWindowResize(width, height);
        }
    }
    virtual std::shared_ptr<Camera> GetCamera() const
    {
        return m_Scene ? m_Scene->GetCamera() : nullptr;
    }

protected:
    std::unique_ptr<Renderer> m_Renderer;
    std::shared_ptr<Scene> m_Scene;
};