#pragma once

#include <memory>
#include "Scene/Scene.h"
#include "Renderer/Renderer.h"
#include "Scene/Screen.h"  // Screen provides static members for width/height

/**
 * @brief Base class representing a self-contained demo/test.
 */
class Test {
public:
    Test() {
        // Create renderer from the Renderer namespace.
        renderer_ = std::make_unique<Renderer>();
        // Create scene from the Scene namespace.
        scene_ = std::make_shared<Scene::Scene>();

        // Initialize the renderer with the initial window dimensions.
        renderer_->Initialize(Screen::s_Width, Screen::s_Height);
    }
    virtual ~Test() = default;

    virtual void OnEnter() {}
    virtual void OnExit() {
        if (renderer_) {
            renderer_.reset();
        }
        if (scene_) {
            scene_->Clear();
        }
    }
    virtual void OnUpdate(float /*deltaTime*/) {}
    virtual void OnRender() {
        if (renderer_ && scene_) {
            renderer_->RenderScene(scene_);
        }
    }
    virtual void OnImGuiRender() {}
    virtual void OnWindowResize(int width, int height) {
        if (renderer_) {
            renderer_->OnWindowResize(width, height);
        }
    }
    virtual std::shared_ptr<Scene::Camera> GetCamera() const {
        return scene_ ? scene_->GetCamera() : nullptr;
    }

protected:
    std::unique_ptr<Renderer> renderer_;
    std::shared_ptr<Scene::Scene> scene_;
};