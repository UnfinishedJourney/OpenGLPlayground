#pragma once
#include "RenderPass.h"
#include "Scene/Scene.h"
#include <memory>
#include "Graphics/Effects/FlipBookEffects/FlipBookEffect.h"

class VisualEffectsPass : public RenderPass {
public:
    VisualEffectsPass() = default;
    ~VisualEffectsPass() = default;

    void Execute(const std::shared_ptr<Scene::Scene>& scene) override;
    void AddFlipbookEffect(std::shared_ptr<FlipbookEffect> effect);

    void Update(double currentTime);

private:
    std::vector<std::shared_ptr<FlipbookEffect>> m_FlipbookEffects;
};