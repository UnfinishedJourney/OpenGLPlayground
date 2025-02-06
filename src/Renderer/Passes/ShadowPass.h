#pragma once
#include <memory>
#include "RenderPass.h"
#include "Scene/Scene.h"
#include "Graphics/Buffers/ShadowMap.h"
#include "Graphics/Shaders/Shader.h" // your own shader wrapper class

class ShadowPass : public RenderPass {
public:
    // shadowResolution is the width/height of the shadow map.
    ShadowPass(const std::shared_ptr<Scene>& scene, GLsizei shadowResolution);
    ~ShadowPass();

    // Render the scene from the light’s POV into the shadow map.
    void Execute(const std::shared_ptr<Scene>& scene) override;

    // For this pass, UpdateFramebuffer is not used.
    void UpdateFramebuffer(std::shared_ptr<FrameBuffer> framebuffer) override {}

    // Retrieve the ShadowMap so the texture can be used in the main pass.
    std::shared_ptr<ShadowMap> GetShadowMap() const { return m_ShadowMap; }

private:
    std::shared_ptr<ShadowMap> m_ShadowMap;
    std::shared_ptr<Shader> m_ShadowShader; // used during shadow pass
    // Matrices computed from the light’s position/direction
    glm::mat4 m_ShadowMatrix;
};