#pragma once

#include "Mesh.h"
#include "Material.h"
#include "Transform.h"

class RenderObject {
public:
    RenderObject(std::shared_ptr<MeshBuffer> meshComp, std::shared_ptr<Material> mat, std::unique_ptr<Transform> transform)
        : m_MeshBuffer(meshComp), m_Material(mat), m_Transform(std::move(transform))
    {}

    virtual void Render() {
        m_MeshBuffer->Bind();
        m_Material->Bind();
        glm::mat4 mvp = FrameData::s_Projection * FrameData::s_View * m_Transform->GetModelMatrix();
        m_Material->GetShader()->SetUniform("u_MVP", mvp);
        GLCall(glDrawElements(GL_TRIANGLES, m_MeshBuffer->GetNVerts(), GL_UNSIGNED_INT, nullptr));
    }

    std::unique_ptr<Transform>& GetTransform()
    {
        return m_Transform;
    }

    virtual void Update(float deltaTime) 
    {
        return;
    }

protected:
    std::shared_ptr<MeshBuffer> m_MeshBuffer;
    std::shared_ptr<Material> m_Material;
    std::unique_ptr<Transform> m_Transform;
};

class StaticObject : public RenderObject {
public:
    StaticObject(std::shared_ptr<MeshBuffer> meshComp, std::shared_ptr<Material> mat, std::unique_ptr<Transform> transform)
        : RenderObject(meshComp, mat, std::move(transform))
    {}

    void Update(float deltaTime) override {
        return;
    }
};