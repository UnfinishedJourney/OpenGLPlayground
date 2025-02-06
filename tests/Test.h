#pragma once
#include <memory>
#include "Scene/Scene.h"
#include "Renderer/Renderer.h"
#include "Scene/Screen.h"

/**
 * @class Test
 * @brief Base class representing a self-contained demo.
 */
class Test
{
public:
    Test()
    {
        // Build a renderer
        m_Renderer = std::make_unique<Renderer>();
        // Build a scene
        m_Scene = std::make_shared<Scene>();

        // Initialize the renderer with initial window size
        m_Renderer->Initialize(Screen::s_Width, Screen::s_Height);
    }

    virtual ~Test() = default;

    // Called when becoming the active test
    virtual void OnEnter() {}

    // Called once when we leave this test
    virtual void OnExit()
    {
        // Cleanup resources
        if (m_Renderer) {
            m_Renderer.reset();
        }
        if (m_Scene) {
            m_Scene->Clear();
        }
    }

    // Called every frame to update logic
    virtual void OnUpdate(float /*deltaTime*/) {}

    // Called every frame to render
    virtual void OnRender()
    {
        if (m_Renderer && m_Scene) {
            m_Renderer->RenderScene(m_Scene);
        }
    }

    // Called every frame to render ImGui
    virtual void OnImGuiRender() {}

    // Window resized
    virtual void OnWindowResize(int width, int height)
    {
        if (m_Renderer) {
            m_Renderer->OnWindowResize(width, height);
        }
    }

    // Retrieve camera
    virtual std::shared_ptr<Camera> GetCamera() const
    {
        return m_Scene ? m_Scene->GetCamera() : nullptr;
    }

protected:
    std::unique_ptr<Renderer> m_Renderer;
    std::shared_ptr<Scene>    m_Scene;
};