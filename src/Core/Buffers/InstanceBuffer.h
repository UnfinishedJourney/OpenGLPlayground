#pragma once

#include "Mesh.h"

class InstanceBuffer {
public:
    InstanceBuffer(std::shared_ptr<MeshBuffer> meshBuffer, size_t instanceCount)
        : m_MeshBuffer(meshBuffer), m_InstanceCount(instanceCount)
    {
        InitInstanceBuffer();
    }

    void RenderInstances() {
        m_MeshBuffer->Bind();
        //m_InstanceBuffer->Bind();

        GLCall(glDrawElementsInstanced(GL_TRIANGLES, m_MeshBuffer->GetNVerts(), GL_UNSIGNED_INT, 0, m_InstanceCount));

        //m_MeshBuffer->Unbind();
        //m_InstanceBuffer->Unbind();
    }

private:
    std::shared_ptr<MeshBuffer> m_MeshBuffer;
    size_t m_InstanceCount;
    std::unique_ptr<VertexArray> m_InstanceBuffer;

    void InitInstanceBuffer();
};