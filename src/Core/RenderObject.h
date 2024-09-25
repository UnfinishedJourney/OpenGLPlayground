#pragma once

#include "Mesh.h"
#include "Material.h"
#include "Transform.h"
#include "InstanceBuffer.h"

class RenderObject {
public:
    RenderObject(std::shared_ptr<MeshBuffer> meshBuffer, std::shared_ptr<Material> mat, std::unique_ptr<Transform> transform)
        : m_Material(mat), m_Transform(std::move(transform))
    {
        m_MeshBuffers.push_back(meshBuffer);
    }

    RenderObject(std::vector<std::shared_ptr<MeshBuffer>> meshBuffers, std::shared_ptr<Material> mat, std::unique_ptr<Transform> transform)
        : m_MeshBuffers(meshBuffers), m_Material(mat), m_Transform(std::move(transform))
    {
    }

    virtual void Render() {
        for (auto& meshBuffer : m_MeshBuffers)
        {
            meshBuffer->Bind();
            m_Material->Bind();
            glm::mat4 mvp = FrameData::s_Projection * FrameData::s_View * m_Transform->GetModelMatrix();
            m_Material->GetShader()->SetUniform("u_MVP", mvp);
            GLCall(glDrawElements(GL_TRIANGLES, meshBuffer->GetNVerts(), GL_UNSIGNED_INT, nullptr));
        }
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
    std::vector<std::shared_ptr<MeshBuffer>> m_MeshBuffers;
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

class InstancedRenderObject {
public:

    InstancedRenderObject(std::shared_ptr<MeshBuffer> meshBuffer, std::shared_ptr<Material> mat, std::unique_ptr<Transform> transform, size_t nInstances = 100)
        : m_Material(mat), m_Transform(std::move(transform))
    {
        m_InstanceBuffer = std::make_shared<InstanceBuffer>(meshBuffer, nInstances);
    }

    virtual void Render() {
        m_InstanceBuffer->RenderInstances();
        m_Material->Bind();
        glm::mat4 mvp = FrameData::s_Projection * FrameData::s_View * m_Transform->GetModelMatrix();
        m_Material->GetShader()->SetUniform("u_MVP", mvp);
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
    std::shared_ptr<InstanceBuffer> m_InstanceBuffer;
    std::shared_ptr<Material> m_Material;
    std::unique_ptr<Transform> m_Transform;
};