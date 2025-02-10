#include "ShadowPass.h"
#include <glad/glad.h>
#include "Utilities/Logger.h" // if you use a logger
#include <glm/gtc/matrix_transform.hpp>
#include "Renderer/Batch.h"
#include "Resources/ShaderManager.h"

ShadowPass::ShadowPass(const std::shared_ptr<Scene>& scene, GLsizei shadowResolution)
{
    m_ShadowMap = std::make_shared<Graphics::ShadowMap>(shadowResolution, shadowResolution);
    auto& shaderManager = Graphics::ShaderManager::GetInstance();
    m_ShadowShader = shaderManager.GetShader("basicShadowMap");

    auto lightManager = scene->GetLightManager();

    glm::mat4 lightView = lightManager->ComputeLightView(0);
    glm::mat4 lightProj = lightManager->ComputeLightProj(0);

    //glm::vec4 d = glm::vec4(0.0, 1.0, 0.0, 1.0);
    //d = lightProj * lightView * d;
    m_ShadowMatrix = lightProj * lightView;

    //auto lightManager = scene->GetLightManager();
    //auto camera = scene->GetCamera();
    //auto camView = camera->GetViewMatrix();
    //auto camProj = camera->GetProjectionMatrix();


    //std::array<glm::vec3, 8> corners;

    //// 1) Inverse of the camera's VP
    //glm::mat4 invVP = glm::inverse(camProj * camView);

    //// 2) Clip-space corners: x,y in [-1,1], z in [0,1] for a typical perspective
    ////    We'll define them in NDC: near plane z=0, far plane z=1
    //// near plane:
    //glm::vec4 ndc[8] = {
    //    // near
    //    glm::vec4(-1, -1, 0, 1),
    //    glm::vec4(+1, -1, 0, 1),
    //    glm::vec4(-1, +1, 0, 1),
    //    glm::vec4(+1, +1, 0, 1),
    //    // far
    //    glm::vec4(-1, -1, 1, 1),
    //    glm::vec4(+1, -1, 1, 1),
    //    glm::vec4(-1, +1, 1, 1),
    //    glm::vec4(+1, +1, 1, 1)
    //};

    //// 3) Transform each corner by invVP to get world-space
    //for (int i = 0; i < 8; i++)
    //{
    //    glm::vec4 ws = invVP * ndc[i];
    //    // perspective divide
    //    ws /= ws.w;

    //    corners[i] = glm::vec3(ws);
    //}

    //glm::vec3 minB = glm::vec3(FLT_MAX);
    //glm::vec3 maxB = glm::vec3(FLT_MIN);


    //for (int i = 0; i < 8; i++)
    //{
    //    minB = glm::min(minB, corners[i]);
    //    maxB = glm::max(maxB, corners[i]);
    //}

    //auto bb = scene->ComputeWorldBoundingBox();

    //minB = glm::max(minB, bb.min_);
    //maxB = glm::min(maxB, bb.max_);

    //lightManager->SetBoundingBox(minB, maxB);

    //glm::mat4 lightView = lightManager->ComputeLightView(0);
    //glm::mat4 lightProj = lightManager->ComputeLightProj(0);
}

ShadowPass::~ShadowPass() {
    // Unique pointers clean up automatically.
}

void ShadowPass::Execute(const std::shared_ptr<Scene>& scene) {

    if (m_BCalculated)
        return;

    // --- PASS ONE: render scene depth from light's POV ---
    m_ShadowMap->BindForWriting();
    glClear(GL_DEPTH_BUFFER_BIT);

    // Set face culling to front-face to reduce self-shadowing artifacts.
    glCullFace(GL_FRONT);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.f, 2.f);

    m_ShadowShader->Bind();
    // Build the combined light-space matrix:
    // ShadowMatrix = Bias * LightProj * LightView
    // (The bias matrix converts clip coords [-1,1] to [0,1].)
    
    m_ShadowShader->SetUniform("u_ShadowMatrix", m_ShadowMatrix);

    const auto& staticBatches = scene->GetStaticBatches();
    for (auto& batch : staticBatches)
    {
        batch->Render();
    }

    // Restore state: revert culling order and unbind FBO.
    glDisable(GL_POLYGON_OFFSET_FILL);
    glCullFace(GL_BACK);
    m_ShadowMap->Unbind();

    GLuint depthTexID = m_ShadowMap->GetDepthTexture();
    GLsizei w = m_ShadowMap->GetWidth();
    GLsizei h = m_ShadowMap->GetHeight();

    glBindTextureUnit(10, depthTexID);

    m_BCalculated = true;
    // Wrap the existing GL texture ID
    //auto texture = std::make_shared<ExistingMapTexture>(depthTexID, w, h);
    //return texture;

    //m_ShadowMap->GetDepthTexture();
}