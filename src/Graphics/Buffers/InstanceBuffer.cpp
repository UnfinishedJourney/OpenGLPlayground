#include "Graphics/Buffers/InstanceBuffer.h"
#include "gtc/matrix_transform.hpp"

void InstanceBuffer::InitInstanceBuffer() {
    return;
    std::vector<glm::mat4> instanceMatrices(m_InstanceCount);

    unsigned int gridSize = static_cast<unsigned int>(std::sqrt(m_InstanceCount));
    float spacing = 2.0f; 

    /*unsigned int index = 0;
    for (unsigned int x = 0; x < gridSize; ++x) {
        for (unsigned int z = 0; z < gridSize; ++z) {
            if (index >= m_InstanceCount) break;

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(
                (x - gridSize / 2) * spacing,
                0.0f,
                (z - gridSize / 2) * spacing
            ));
            instanceMatrices[index++] = model;
        }
    }

    m_InstanceBuffer = std::make_unique<VertexBuffer>(instanceMatrices.data(), instanceMatrices.size() * sizeof(glm::mat4));*/

    m_MeshBuffer->GetVAO()->Bind();

    //m_InstanceBuffer->Bind();

    GLsizei vec4Size = sizeof(glm::vec4);
    GLsizei mat4Size = sizeof(glm::mat4);

    for (unsigned int i = 0; i < 4; i++) {
        glEnableVertexAttribArray(3 + i);
        glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, mat4Size, (void*)(vec4Size * i));
        glVertexAttribDivisor(3 + i, 1);
    }

    //m_InstanceBuffer->Unbind();
    m_MeshBuffer->GetVAO()->Unbind();
}