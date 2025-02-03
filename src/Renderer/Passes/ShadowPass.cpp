#include "ShadowPass.h"
#include <glad/glad.h>
#include "Utilities/Logger.h" // if you use a logger
#include <glm/gtc/matrix_transform.hpp>
#include "Renderer/Batch.h"
#include "Resources/ShaderManager.h"

ShadowPass::ShadowPass(GLsizei shadowResolution, const glm::mat4& lightView, const glm::mat4& lightProj):
    m_LightView(lightView),
    m_LightProj(lightProj) 
{
    m_ShadowMap = std::make_shared<ShadowMap>(shadowResolution, shadowResolution);
    auto& shaderManager = ShaderManager::GetInstance();
    m_ShadowShader = shaderManager.GetShader("basicShadowMap");;
}

ShadowPass::~ShadowPass() {
    // Unique pointers clean up automatically.
}

void ShadowPass::Execute(const std::shared_ptr<Scene>& scene) {
    // --- PASS ONE: render scene depth from light's POV ---
    m_ShadowMap->BindForWriting();
    glClear(GL_DEPTH_BUFFER_BIT);

    // Set face culling to front-face to reduce self-shadowing artifacts.
    glCullFace(GL_FRONT);

    m_ShadowShader->Bind();
    // Build the combined light-space matrix:
    // ShadowMatrix = Bias * LightProj * LightView
    // (The bias matrix converts clip coords [-1,1] to [0,1].)
    glm::mat4 bias = glm::mat4(
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.5f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f
    );
    glm::mat4 shadowMatrix = bias * m_LightProj * m_LightView;
    m_ShadowShader->SetUniform("u_ShadowMatrix", shadowMatrix);

    const auto& staticBatches = scene->GetStaticBatches();
    for (auto& batch : staticBatches)
    {
        batch->Render();
    }

    // Restore state: revert culling order and unbind FBO.
    glCullFace(GL_BACK);
    m_ShadowMap->Unbind();
}