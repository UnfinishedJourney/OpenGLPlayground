#pragma once

#include <memory>
#include "Scene/Camera.h"
#include "Renderer/Renderer.h"
#include "Scene/Scene.h"
#include "Scene/Screen.h"

/**
 * @class Test
 * @brief Base class representing a self-contained demo or scene.
 */
class Test
{
public:
    /**
     * @brief Default constructor. Initializes a Renderer and an empty Scene with a default camera.
     */
    Test()
    {
        m_Renderer = std::make_unique<Renderer>();
        m_Scene = std::make_shared<Scene>();

        // Initialize renderer with the current screen size.
        m_Renderer->Initialize(Screen::s_Width, Screen::s_Height);

        // Attach a default camera to the scene.
        auto defaultCamera = std::make_shared<Camera>();
        m_Scene->SetCamera(defaultCamera);
    }

    /**
     * @brief Virtual destructor.
     */
    virtual ~Test() = default;

    /**
     * @brief Called when this Test becomes the active test.
     */
    virtual void OnEnter() {}

    /**
     * @brief Called when this Test is about to exit. Use for cleanup.
     */
    virtual void OnExit()
    {
        // By default, reset the renderer and clear the scene.
        m_Renderer.reset();
        if (m_Scene)
        {
            m_Scene->Clear();
        }
    }

    /**
     * @brief Called every frame to update logic.
     *
     * @param deltaTime Time elapsed since the previous frame, in seconds.
     */
    virtual void OnUpdate(float /*deltaTime*/) {}

    /**
     * @brief Called every frame to perform rendering.
     */
    virtual void OnRender()
    {
        if (m_Renderer && m_Scene)
        {
            m_Renderer->RenderScene(m_Scene);
        }
    }

    /**
     * @brief Called every frame to render ImGui elements for this test.
     */
    virtual void OnImGuiRender() {}

    /**
     * @brief Called when the window is resized.
     *
     * @param width  New width of the window.
     * @param height New height of the window.
     */
    virtual void OnWindowResize(int width, int height)
    {
        if (m_Renderer)
        {
            m_Renderer->OnWindowResize(width, height);
        }
    }

    /**
     * @brief Retrieves the camera used by this test’s scene.
     *
     * @return A shared pointer to the camera, or nullptr if none.
     */
    virtual std::shared_ptr<Camera> GetCamera() const
    {
        return m_Scene ? m_Scene->GetCamera() : nullptr;
    }

protected:
    /**
     * @brief A unique pointer to the Renderer instance for this test.
     */
    std::unique_ptr<Renderer> m_Renderer;

    /**
     * @brief A shared pointer to the Scene instance for this test.
     */
    std::shared_ptr<Scene> m_Scene;
};