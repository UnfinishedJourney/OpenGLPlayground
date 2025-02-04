#pragma once
#include <memory>
#include "RenderPass.h"
#include "Scene/Scene.h"
#include "Graphics/Buffers/ShadowMap.h"
#include "Graphics/Shaders/Shader.h" // your own shader wrapper class

class ShadowPass : public RenderPass {
public:
    // shadowResolution is the width/height of the shadow map.
    ShadowPass(GLsizei shadowResolution);
    ~ShadowPass();

    // Render the scene from the light’s POV into the shadow map.
    void Execute(const std::shared_ptr<Scene>& scene) override;

    // For this pass, UpdateFramebuffer is not used.
    void UpdateFramebuffer(std::shared_ptr<FrameBuffer> framebuffer) override {}

    // Retrieve the ShadowMap so the texture can be used in the main pass.
    std::shared_ptr<ShadowMap> GetShadowMap() const { return m_ShadowMap; }

    // Set the light’s view/projection matrices and other uniforms.
    void SetLightMatrices(const glm::mat4& lightView, const glm::mat4& lightProj) {
        m_LightView = lightView;
        m_LightProj = lightProj;
    }

private:
    std::shared_ptr<ShadowMap> m_ShadowMap;
    std::shared_ptr<Shader> m_ShadowShader; // used during shadow pass
    // Matrices computed from the light’s position/direction
    glm::mat4 m_LightView;
    glm::mat4 m_LightProj;
};