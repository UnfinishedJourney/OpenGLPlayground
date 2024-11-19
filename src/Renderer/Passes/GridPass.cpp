#include "GridPass.h"
#include "Resources/ResourceManager.h"
#include "Utilities/Logger.h"
#include <glad/glad.h>

GridPass::GridPass(std::shared_ptr<FrameBuffer> framebuffer, const std::shared_ptr<Scene>& scene)
    : m_Framebuffer(framebuffer)
{
    auto& resourceManager = ResourceManager::GetInstance();

    // Create a quad mesh covering the entire grid area
    auto gridMesh = resourceManager.GetMesh("quad");
    MeshLayout gridMeshLayout = {
        true,  // Positions
        false, // Normals
        false, // Tangents
        false, // Bitangents
        {}     // Texture Coordinates
    };

    m_GridMeshBuffer = resourceManager.GetMeshBuffer("quad", gridMeshLayout);
}

GridPass::~GridPass()
{
}

void GridPass::Execute(const std::shared_ptr<Scene>& scene)
{
    m_Framebuffer->Bind();

    // Enable blending for transparency
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set depth function to ensure grid is rendered beneath other objects
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    auto& resourceManager = ResourceManager::GetInstance();
    resourceManager.BindShader("grid");

    auto shader = resourceManager.GetCurrentlyBoundShader();
    if (!shader) {
        Logger::GetLogger()->error("Grid shader not bound.");
        return;
    }

    scene->BindFrameDataUBO();
    // Set additional uniforms if needed (e.g., grid parameters)

    // Render the grid mesh
    m_GridMeshBuffer->Bind();
    glDrawArrays(GL_TRIANGLES, 0, m_GridMeshBuffer->GetIndexCount());
    m_GridMeshBuffer->Unbind();

    // Reset render states
    glDisable(GL_BLEND);
    glDepthFunc(GL_LESS);

    m_Framebuffer->Unbind();
}

void GridPass::UpdateFramebuffer(std::shared_ptr<FrameBuffer> framebuffer)
{
    m_Framebuffer = framebuffer;
}