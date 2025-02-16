#include "VisualEffectsPass.h"

void VisualEffectsPass::Execute(const std::shared_ptr<Scene::Scene>& scene) {
    // Render each flipbook effect.
    for (auto& effect : flipbookEffects_) {
        effect->Render();
    }
}

void VisualEffectsPass::AddFlipbookEffect(std::shared_ptr<FlipbookEffect> effect) {
    flipbookEffects_.push_back(effect);
}

void VisualEffectsPass::Update(double currentTime) {
    for (auto& effect : flipbookEffects_) {
        effect->Update(currentTime);
    }
}