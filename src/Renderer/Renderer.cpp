#include "Renderer/Renderer.h"
#include <iostream>
#include "Utilities/Logger.h"
#include "Scene/FrameData.h"
#include "Utilities/Utility.h" 

Renderer::Renderer()
{
    auto logger = Logger::GetLogger();
    logger->info("Initializing Renderer.");

    try {
        m_ResourceManager = std::make_unique<ResourceManager>();
        logger->info("ResourceManager initialized successfully.");
    }
    catch (const std::exception& e) {
        logger->critical("Failed to initialize ResourceManager: {}", e.what());
        throw; 
    }
}

void Renderer::Render(const std::shared_ptr<RenderObject>& renderObject) const
{
    auto logger = Logger::GetLogger();
    logger->debug("Starting Render process for RenderObject: {}", "");

    if (!renderObject) {
        logger->error("RenderObject is nullptr. Skipping rendering.");
        return;
    }

    try {
        logger->debug("Binding Shader: {}", renderObject->m_ShaderName);
        m_ResourceManager->BindShader(renderObject->m_ShaderName);
    }
    catch (const std::exception& e) {
        logger->error("Failed to bind shader '{}': {}", renderObject->m_ShaderName, e.what());
        return;
    }

    try {
        logger->debug("Binding Material: {}", renderObject->m_MaterialName);
        m_ResourceManager->BindMaterial(renderObject->m_MaterialName);
    }
    catch (const std::exception& e) {
        logger->error("Failed to bind material '{}': {}", renderObject->m_MaterialName, e.what());
        return;
    }

    glm::mat4 modelMatrix = renderObject->m_Transform->GetModelMatrix();
    glm::mat4 mvp = FrameData::s_Projection * FrameData::s_View * modelMatrix;
    logger->debug("Calculated MVP matrix for RenderObject '{}'.", "");

    try {
        logger->debug("Setting uniform 'u_MVP' for Shader: {}", renderObject->m_ShaderName);
        m_ResourceManager->SetUniform("u_MVP", mvp);
    }
    catch (const std::exception& e) {
        logger->error("Failed to set uniform 'u_MVP' for shader '{}': {}", renderObject->m_ShaderName, e.what());
        return;
    }

    try {
        logger->debug("Binding MeshBuffer for RenderObject '{}'.", "");
        renderObject->m_MeshBuffer->Bind();
    }
    catch (const std::exception& e) {
        logger->error("Failed to bind MeshBuffer for RenderObject '{}': {}", "", e.what());
        return;
    }

    try {
        logger->debug("Executing glDrawElements for RenderObject '{}'.", "");
        GLCall(glDrawElements(GL_TRIANGLES, renderObject->m_MeshBuffer->GetNVerts(), GL_UNSIGNED_INT, nullptr));
        logger->debug("glDrawElements executed successfully for RenderObject '{}'.", "");
    }
    catch (const std::exception& e) {
        logger->error("glDrawElements failed for RenderObject '{}': {}", "", e.what());
    }

    logger->debug("Render process completed for RenderObject '{}'.", "");
}

void Renderer::RenderSkybox(const std::shared_ptr<MeshBuffer>& meshBuffer, const std::string& textureName, const std::string& shaderName) const
{
    auto logger = Logger::GetLogger();
    logger->debug("Starting RenderSkybox process.");

    if (!meshBuffer) {
        logger->error("Skybox MeshBuffer is nullptr. Skipping skybox rendering.");
        return;
    }

    try {
        logger->debug("Binding Shader: {}", shaderName);
        m_ResourceManager->BindShader(shaderName);
    }
    catch (const std::exception& e) {
        logger->error("Failed to bind skybox shader '{}': {}", shaderName, e.what());
        return;
    }

    try {
        logger->debug("Binding CubeMapTexture: {}", textureName);
        m_ResourceManager->BindCubeMapTexture(textureName, 0); // Binding to texture unit 0
    }
    catch (const std::exception& e) {
        logger->error("Failed to bind cube map texture '{}': {}", textureName, e.what());
        return;
    }

    logger->debug("Adjusting OpenGL state for skybox rendering.");
    glDepthFunc(GL_LEQUAL); // Change depth function so skybox is rendered behind all objects
    glDepthMask(GL_FALSE);  // Disable depth writing

    try {
        logger->debug("Setting uniform 'u_MVP' for Shader: {}", shaderName);
        auto mvp = FrameData::s_Projection * FrameData::s_View;
        m_ResourceManager->SetUniform("u_MVP", mvp);
    }
    catch (const std::exception& e) {
        logger->error("Failed to set uniforms for skybox shader '{}': {}", shaderName, e.what());
        // Restore OpenGL state before returning
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);
        return;
    }

    try {
        logger->debug("Binding MeshBuffer for Skybox.");
        meshBuffer->Bind();
    }
    catch (const std::exception& e) {
        logger->error("Failed to bind MeshBuffer for Skybox: {}", e.what());
        // Restore OpenGL state before returning
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);
        return;
    }

    try {
        logger->debug("Executing glDrawElements for Skybox.");
        GLCall(glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(meshBuffer->GetNVerts()), GL_UNSIGNED_INT, nullptr));
        logger->debug("glDrawElements executed successfully for Skybox.");
    }
    catch (const std::exception& e) {
        logger->error("glDrawElements failed for Skybox: {}", e.what());
    }

    // Unbind MeshBuffer
    meshBuffer->Unbind();

    // Restore OpenGL state
    logger->debug("Restoring OpenGL state after skybox rendering.");
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);

    logger->debug("RenderSkybox process completed.");
}

void Renderer::Clear() const
{
    auto logger = Logger::GetLogger();
    logger->debug("Clearing the screen.");

    try {
        GLCall(glClearColor(0.3f, 0.4f, 0.55f, 1.0f));
        logger->debug("Set clear color to (0.3, 0.4, 0.55, 1.0).");

        GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        logger->debug("Cleared color and depth buffers.");
    }
    catch (const std::exception& e) {
        logger->error("Failed to clear buffers: {}", e.what());
    }

    logger->debug("Screen cleared successfully.");
}