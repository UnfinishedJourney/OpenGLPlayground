#include "Renderer/Renderer.h"
#include <iostream>

Renderer::Renderer()
{
    m_ResourceManager = std::make_unique<ResourceManager>();
}

void Renderer::Render(std::shared_ptr<RenderObject> renderObject) const
{
    m_ResourceManager->BindShader(renderObject->m_ShaderName);
    m_ResourceManager->BindMaterial(renderObject->m_MaterialName);
    glm::mat4 mvp = FrameData::s_Projection * FrameData::s_View * renderObject->m_Transform->GetModelMatrix();
    m_ResourceManager->SetUniform("u_MVP", mvp);
    renderObject->m_MeshBuffer->Bind();
    GLCall(glDrawElements(GL_TRIANGLES, renderObject->m_MeshBuffer->GetNVerts(), GL_UNSIGNED_INT, nullptr));
}

void Renderer::Clear() const
{
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    GLCall(glClearColor(0.3f, 0.4f, 0.55f, 1.0f));
}