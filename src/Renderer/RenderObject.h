#pragma once

#include "Graphics/Meshes/Mesh.h"
#include "Scene/Transform.h"

#include <memory>
#include <string>

class MeshBuffer;

class RenderObject {

public:
    RenderObject(std::shared_ptr<MeshBuffer> meshBuffer, std::string materialName, std::string shaderName, std::unique_ptr<Transform> transform)
        : m_MeshBuffer(meshBuffer), m_MaterialName(materialName), m_ShaderName(shaderName), m_Transform(std::move(transform))
    {
    }

    //virtual void Render() {
    //    GLCall(glDrawElements(GL_TRIANGLES, meshBuffer->GetNVerts(), GL_UNSIGNED_INT, nullptr));
    //    for (auto& meshBuffer : m_MeshBuffers)
    //    {
    //        meshBuffer->Bind();
    //        m_Material->Bind();
    //        glm::mat4 mvp = FrameData::s_Projection * FrameData::s_View * m_Transform->GetModelMatrix();
    //        m_Material->GetShader()->SetUniform("u_MVP", mvp);
    //        GLCall(glDrawElements(GL_TRIANGLES, meshBuffer->GetNVerts(), GL_UNSIGNED_INT, nullptr));
    //    }
    //}

    //virtual void Update(float deltaTime) 
    //{
    //    return;
    //}
    std::string m_MaterialName;
    std::string m_ShaderName;
    std::shared_ptr<MeshBuffer> m_MeshBuffer;
    std::unique_ptr<Transform> m_Transform;
};