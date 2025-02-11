#include "VisualEffectsPass.h"

void VisualEffectsPass::Execute(const std::shared_ptr<Scene::Scene>& scene) {
    // Assuming the default framebuffer is currently bound, or you bind one as needed
    for (auto& effect : m_FlipbookEffects) {
        effect->Render();
    }
}

void VisualEffectsPass::AddFlipbookEffect(std::shared_ptr<FlipbookEffect> effect) {
    m_FlipbookEffects.push_back(effect);
}

void VisualEffectsPass::Update(double currentTime) {
    for (auto& effect : m_FlipbookEffects) {
        effect->Update(currentTime);
    }
}