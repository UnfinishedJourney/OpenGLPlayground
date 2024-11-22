//#include "InstanceBuffer.h"
//#include "Utilities/Logger.h"
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <stdexcept>
//#include <cmath>
//
//InstanceBuffer::InstanceBuffer(std::shared_ptr<MeshBuffer> meshBuffer, size_t instanceCount)
//    : m_MeshBuffer(std::move(meshBuffer)), m_InstanceCount(instanceCount),
//    m_InstanceVBO(nullptr, 0) // Placeholder, will initialize in InitInstanceBuffer
//{
//    InitInstanceBuffer();
//}
//
//void InstanceBuffer::InitInstanceBuffer() {
//    // Generate instance transformation matrices
//    std::vector<glm::mat4> instanceMatrices(m_InstanceCount);
//
//    unsigned int gridSize = static_cast<unsigned int>(std::sqrt(m_InstanceCount));
//    float spacing = 2.0f;
//
//    unsigned int index = 0;
//    for (unsigned int x = 0; x < gridSize; ++x) {
//        for (unsigned int z = 0; z < gridSize; ++z) {
//            if (index >= m_InstanceCount) break;
//
//            glm::mat4 model = glm::mat4(1.0f);
//            model = glm::translate(model, glm::vec3(
//                (x - gridSize / 2) * spacing,
//                0.0f,
//                (z - gridSize / 2) * spacing
//            ));
//            instanceMatrices[index++] = model;
//        }
//    }
//
//    m_InstanceVBO = VertexBuffer(instanceMatrices.data(), instanceMatrices.size() * sizeof(glm::mat4));
//
//    Logger::GetLogger()->info("Initialized InstanceBuffer with {} instances.", m_InstanceCount);
//
//    // Configure vertex attributes for the instance data
//    GLuint vaoID = m_MeshBuffer->GetVAO(); // Ensure GetVAO() returns GLuint
//    if (vaoID == 0) {
//        Logger::GetLogger()->error("MeshBuffer does not have a valid VAO.");
//        throw std::runtime_error("Invalid VAO in MeshBuffer.");
//    }
//    glBindVertexArray(vaoID);
//
//    GLsizei vec4Size = sizeof(glm::vec4);
//    GLsizei mat4Size = sizeof(glm::mat4);
//
//    for (unsigned int i = 0; i < 4; i++) {
//        GLuint attribIndex = 3 + i; // Assuming the first 3 attributes are already used (e.g., position, normal, texCoords)
//        glEnableVertexAttribArray(attribIndex);
//        glVertexAttribPointer(attribIndex, 4, GL_FLOAT, GL_FALSE, mat4Size, (void*)(vec4Size * i));
//        glVertexAttribDivisor(attribIndex, 1); // Tell OpenGL this is an instanced vertex attribute
//    }
//
//    glBindVertexArray(0);
//
//    Logger::GetLogger()->info("Configured instanced vertex attributes for InstanceBuffer.");
//}
//
//void InstanceBuffer::RenderInstances() const {
//    m_MeshBuffer->Bind();
//
//    // Draw instanced elements
//    glDrawElementsInstanced(GL_TRIANGLES, m_MeshBuffer->GetIndexCount(), GL_UNSIGNED_INT, 0, static_cast<GLsizei>(m_InstanceCount));
//
//    m_MeshBuffer->Unbind();
//
//    Logger::GetLogger()->debug("Rendered {} instances.", m_InstanceCount);
//}