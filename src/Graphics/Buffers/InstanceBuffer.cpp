#include "InstanceBuffer.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>
#include <cmath>

InstanceBuffer::InstanceBuffer(std::shared_ptr<graphics::MeshBuffer> meshBuffer, size_t instanceCount)
    : meshBuffer_(meshBuffer), instanceCount_(instanceCount)
{
    InitInstanceBuffer();
}

void InstanceBuffer::RenderInstances() {
    meshBuffer_->Bind();
    GLCall(glDrawElementsInstanced(GL_TRIANGLES, meshBuffer_->GetVertexCount(), GL_UNSIGNED_INT, 0, instanceCount_));
}

void InstanceBuffer::InitInstanceBuffer() {
    // Example: setup instanced attributes if needed.
    // Here we simply enable attribute locations 3-6 with a divisor of 1.
    std::vector<glm::mat4> instanceMatrices(instanceCount_);
    // Setup instanceMatrices as needed...

    meshBuffer_->Bind();

    GLsizei vec4Size = sizeof(glm::vec4);
    GLsizei mat4Size = sizeof(glm::mat4);
    for (unsigned int i = 0; i < 4; i++) {
        glEnableVertexAttribArray(3 + i);
        glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, mat4Size, (void*)(vec4Size * i));
        glVertexAttribDivisor(3 + i, 1);
    }
    meshBuffer_->Unbind();
}