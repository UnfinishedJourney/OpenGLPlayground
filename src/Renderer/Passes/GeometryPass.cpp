//#include "GeometryPass.h"
//#include "ResourceManager.h"
//
//GeometryPass::GeometryPass(Renderer& renderer)
//    : m_Renderer(renderer)
//{
//    // Configure render state for geometry pass
//    m_RenderState.SetCullFace(true, GL_BACK);
//    m_RenderState.SetDepthTest(true);
//    m_RenderState.SetBlend(false);
//    m_RenderState.SetDepthMask(true);
//}
//
//void GeometryPass::Setup()
//{
//    // Apply render state
//    m_RenderState.Apply();
//
//    // Bind the scene framebuffer
//    m_Renderer.GetSceneFrameBuffer()->Bind();
//
//    // Clear the framebuffer
//    m_Renderer.Clear(0.1f, 0.1f, 0.1f, 1.0f);
//}
//
//void GeometryPass::Render()
//{
//    // Update frame data UBO
//    m_Renderer.UpdateFrameDataUBO();
//
//    // Build and render batches
//    m_Renderer.GetBatchManager().BuildBatches();
//
//    const auto& batches = m_Renderer.GetBatchManager().GetBatches();
//    for (const auto& batch : batches) {
//        const auto& renderObjects = batch->GetRenderObjects();
//        if (renderObjects.empty()) {
//            continue;
//        }
//
//        // Bind shader and material
//        m_Renderer.BindShaderAndMaterial(batch->GetShaderName(), batch->GetMaterialName());
//
//        for (const auto& renderObject : renderObjects) {
//            glm::mat4 modelMatrix = renderObject->GetTransform()->GetModelMatrix();
//            glm::mat3 normalMatrix = renderObject->GetTransform()->GetNormalMatrix();
//
//            ResourceManager::GetInstance().SetUniform("u_Model", modelMatrix);
//            ResourceManager::GetInstance().SetUniform("u_NormalMatrix", normalMatrix);
//
//            // Render the object
//            renderObject->GetMeshBuffer()->Bind();
//            renderObject->GetMeshBuffer()->Draw();
//            renderObject->GetMeshBuffer()->Unbind();
//        }
//    }
//}
//
//void GeometryPass::Teardown()
//{
//    // Unbind the framebuffer
//    m_Renderer.GetSceneFrameBuffer()->Unbind();
//}