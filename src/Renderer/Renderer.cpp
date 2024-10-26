#include "Renderer/Renderer.h"
#include "Utilities/Logger.h"
#include "Scene/FrameData.h"
#include "Utilities/Utility.h" 
#include "Graphics/Buffers/UniformBuffer.h"

Renderer::Renderer()
{
    auto logger = Logger::GetLogger();
    logger->info("Initializing Renderer.");

    //m_ResourceManager = std::make_unique<ResourceManager>("../shaders/metadata.json", "../shaders/config.json");
    m_ResourceManager = std::make_unique<ResourceManager>();
    logger->info("ResourceManager initialized successfully.");

    m_FrameDataUBO = std::make_unique<UniformBuffer>(sizeof(FrameCommonData), 0, GL_DYNAMIC_DRAW);
    logger->info("Created FrameData UBO with binding point 0.");
}

void Renderer::UpdateFrameDataUBO() const
{
    FrameCommonData frameData;
    frameData.view = FrameData::s_View;  
    frameData.proj = FrameData::s_Projection; 
    frameData.cameraPos = glm::vec4(FrameData::s_CameraPos, 1.0f);

    m_FrameDataUBO->SetData(frameData);
}

void Renderer::BindShaderAndMaterial(const std::shared_ptr<RenderObject>& renderObject) const
{
    if (!renderObject) {
        Logger::GetLogger()->error("RenderObject is nullptr.");
        return;
    }

    m_ResourceManager->BindShader(renderObject->m_ShaderName);
    m_ResourceManager->BindMaterial(renderObject->m_MaterialName);

    //if (!m_ResourceManager->BindShader(renderObject->m_ShaderName)) {
    //    Logger::GetLogger()->error("Failed to bind shader '{}'.", renderObject->m_ShaderName);
    //}

    //if (!m_ResourceManager->BindMaterial(renderObject->m_MaterialName)) {
    //    Logger::GetLogger()->error("Failed to bind material '{}'.", renderObject->m_MaterialName);
    //}
}

void Renderer::Render(const std::shared_ptr<RenderObject>& renderObject) const
{
    UpdateFrameDataUBO();

    BindShaderAndMaterial(renderObject);

    glm::mat4 modelMatrix = renderObject->m_Transform->GetModelMatrix();
    glm::mat4 mvp = FrameData::s_Projection * FrameData::s_View * modelMatrix;

    m_ResourceManager->SetUniform("u_MVP", mvp);

    renderObject->m_MeshBuffer->Bind();
    GLCall(glDrawElements(GL_TRIANGLES, renderObject->m_MeshBuffer->GetVertexCount(), GL_UNSIGNED_INT, nullptr));
}

void Renderer::RenderSkybox(const std::shared_ptr<MeshBuffer>& meshBuffer, const std::string& textureName, const std::string& shaderName) const
{
    if (!meshBuffer) {
        Logger::GetLogger()->error("Skybox MeshBuffer is nullptr.");
        return;
    }

    UpdateFrameDataUBO();

    m_ResourceManager->BindShader(shaderName);
    m_ResourceManager->BindCubeMapTexture(textureName, 0);

    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);

    glm::mat4 mvp = FrameData::s_Projection * FrameData::s_View;
    m_ResourceManager->SetUniform("u_MVP", mvp);

    meshBuffer->Bind();
    GLCall(glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(meshBuffer->GetVertexCount()), GL_UNSIGNED_INT, nullptr));

    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
}

void Renderer::Clear() const
{
    GLCall(glClearColor(0.3f, 0.4f, 0.55f, 1.0f));
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}