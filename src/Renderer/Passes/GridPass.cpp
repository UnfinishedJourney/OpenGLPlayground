#include "GridPass.h"
#include "Graphics/Shaders/ShaderManager.h"
#include "Resources/MeshManager.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <glad/glad.h>
#include "Graphics/Shaders/Shader.h"

GridPass::GridPass(std::shared_ptr<graphics::FrameBuffer> framebuffer,
    const std::shared_ptr<Scene::Scene>& scene)
    : framebuffer_(framebuffer)
{
    auto& meshManager = graphics::MeshManager::GetInstance();
    MeshLayout gridMeshLayout = { true, false, false, false, {} };
    gridMeshBuffer_ = meshManager.GetMeshBuffer("quad", gridMeshLayout);
}

void GridPass::Execute(const std::shared_ptr<Scene::Scene>& scene) {
    framebuffer_->Bind();
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    auto shader = graphics::ShaderManager::GetInstance().GetShader("grid");
    if (!shader) {
        Logger::GetLogger()->error("GridPass: 'grid' shader not found.");
        return;
    }
    shader->Bind();
    scene->BindFrameDataUBO();

    gridMeshBuffer_->Bind();
    GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
    gridMeshBuffer_->Unbind();

    glDisable(GL_BLEND);
    glDepthFunc(GL_LESS);
    framebuffer_->Unbind();
}

GridPass::~GridPass() {
    // Cleanup via smart pointers.
}

void GridPass::UpdateFramebuffer(std::shared_ptr<graphics::FrameBuffer> framebuffer) {
    framebuffer_ = framebuffer;
}
