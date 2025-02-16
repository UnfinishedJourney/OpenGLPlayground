#include "ShadowPass.h"
#include <glad/glad.h>
#include "Utilities/Logger.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Renderer/Batch.h"
#include "Graphics/Shaders/ShaderManager.h"

ShadowPass::ShadowPass(const std::shared_ptr<Scene::Scene>& scene, GLsizei shadowResolution) {
    shadowMap_ = std::make_shared<graphics::ShadowMap>(shadowResolution, shadowResolution);
    auto& shaderManager = graphics::ShaderManager::GetInstance();
    shadowShader_ = shaderManager.GetShader("basicShadowMap");

    auto lightManager = scene->GetLightManager();
    glm::mat4 lightView = lightManager->ComputeLightView(0);
    glm::mat4 lightProj = lightManager->ComputeLightProj(0);
    shadowMatrix_ = lightProj * lightView;
}

ShadowPass::~ShadowPass() {
    // Smart pointers clean up automatically.
}

void ShadowPass::Execute(const std::shared_ptr<Scene::Scene>& scene) {
    if (calculated_)
        return;

    shadowMap_->BindForWriting();
    glClear(GL_DEPTH_BUFFER_BIT);

    glCullFace(GL_FRONT);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.f, 2.f);

    shadowShader_->Bind();
    shadowShader_->SetUniform("u_ShadowMatrix", shadowMatrix_);

    const auto& staticBatches = scene->GetStaticBatches();
    for (auto& batch : staticBatches) {
        batch->Render();
    }

    glDisable(GL_POLYGON_OFFSET_FILL);
    glCullFace(GL_BACK);
    shadowMap_->Unbind();

    // Bind the depth texture to a texture unit for later use.
    GLuint depthTexID = shadowMap_->GetDepthTexture();
    GLsizei w = shadowMap_->GetWidth();
    GLsizei h = shadowMap_->GetHeight();
    glBindTextureUnit(10, depthTexID);

    calculated_ = true;
}