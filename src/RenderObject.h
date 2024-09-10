#pragma once

#include "Mesh.h"
#include "Material.h"

class RenderObject {
public:
    RenderObject(std::shared_ptr<MeshComponent> meshComp, std::shared_ptr<Material> mat, glm::mat4 modelMatrix)
        : m_MeshComponent(meshComp), m_Material(mat), m_ModelMatrix(modelMatrix)
    {}

    void Draw() {
        m_MeshComponent->Bind();
        m_Material->Bind();
        glm::mat4 mvp = FrameData::s_Projection * FrameData::s_View * m_ModelMatrix;
        m_Material->GetShader()->SetUniformMat4f("u_MVP", mvp);
        GLCall(glDrawElements(GL_TRIANGLES, m_MeshComponent->GetNVerts(), GL_UNSIGNED_INT, nullptr));
    }



private:
    std::shared_ptr<MeshComponent> m_MeshComponent;
    std::shared_ptr<Material> m_Material;
    glm::mat4 m_ModelMatrix;
};