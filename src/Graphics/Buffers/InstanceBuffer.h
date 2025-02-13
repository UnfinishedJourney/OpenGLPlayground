#pragma once

#include <glad/glad.h>
#include <vector>
#include <memory>
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "MeshBuffer.h"
#include "Utilities/Utility.h"

//need to revisit this class
class InstanceBuffer {
public:
    InstanceBuffer(std::shared_ptr<graphics::MeshBuffer> meshBuffer, size_t instanceCount)
        : m_MeshBuffer(meshBuffer), m_InstanceCount(instanceCount)
    {
        InitInstanceBuffer();
    }

    InstanceBuffer(InstanceBuffer&&) noexcept = default;
    InstanceBuffer& operator=(InstanceBuffer&&) noexcept = default;

    InstanceBuffer(const InstanceBuffer&) = delete;
    InstanceBuffer& operator=(const InstanceBuffer&) = delete;

    void RenderInstances() {
        m_MeshBuffer->Bind();
        GLCall(glDrawElementsInstanced(GL_TRIANGLES, m_MeshBuffer->GetVertexCount(), GL_UNSIGNED_INT, 0, m_InstanceCount));
    }

private:
    std::shared_ptr<graphics::MeshBuffer> m_MeshBuffer;
    size_t m_InstanceCount;
    graphics::VertexArray m_InstanceVAO;

    void InitInstanceBuffer();
};
