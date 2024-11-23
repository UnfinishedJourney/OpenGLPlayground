#include "InstanceBuffer.h"
#include "Utilities/Logger.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>
#include <cmath>
#include "Graphics/Buffers/InstanceBuffer.h"

void InstanceBuffer::InitInstanceBuffer() {
    return;
    std::vector<glm::mat4> instanceMatrices(m_InstanceCount);

    unsigned int gridSize = static_cast<unsigned int>(std::sqrt(m_InstanceCount));
    float spacing = 2.0f; 
    m_MeshBuffer->Bind();


    GLsizei vec4Size = sizeof(glm::vec4);
    GLsizei mat4Size = sizeof(glm::mat4);

    for (unsigned int i = 0; i < 4; i++) {
        glEnableVertexAttribArray(3 + i);
        glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, mat4Size, (void*)(vec4Size * i));
        glVertexAttribDivisor(3 + i, 1);
    }

    m_MeshBuffer->Unbind();
}