//#pragma once
//
//#include <glad/glad.h>
//#include <vector>
//#include <memory>
//#include "VertexArray.h"
//#include "VertexBuffer.h"
//#include "MeshBuffer.h"
//
//class InstanceBuffer {
//public:
//    InstanceBuffer(std::shared_ptr<MeshBuffer> meshBuffer, size_t instanceCount);
//    ~InstanceBuffer() = default;
//
//    // Move semantics
//    InstanceBuffer(InstanceBuffer&&) noexcept = default;
//    InstanceBuffer& operator=(InstanceBuffer&&) noexcept = default;
//
//    // Deleted copy constructor and assignment operator
//    InstanceBuffer(const InstanceBuffer&) = delete;
//    InstanceBuffer& operator=(const InstanceBuffer&) = delete;
//
//    void RenderInstances() const;
//
//private:
//    std::shared_ptr<MeshBuffer> m_MeshBuffer;
//    size_t m_InstanceCount;
//    VertexBuffer m_InstanceVBO; 
//
//    void InitInstanceBuffer();
//};