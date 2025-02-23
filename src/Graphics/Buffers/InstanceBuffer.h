#pragma once

#include <glad/glad.h>
#include <memory>
#include <vector>
#include "VertexArray.h"
#include "MeshBuffer.h"

//didn't use this class after many important changes
/**
 * @brief Manages instanced rendering.
 */
class InstanceBuffer {
public:
    InstanceBuffer(std::shared_ptr<graphics::MeshBuffer> meshBuffer, size_t instanceCount);
    InstanceBuffer(InstanceBuffer&&) noexcept = default;
    InstanceBuffer& operator=(InstanceBuffer&&) noexcept = default;

    InstanceBuffer(const InstanceBuffer&) = delete;
    InstanceBuffer& operator=(const InstanceBuffer&) = delete;

    void RenderInstances();

private:
    std::shared_ptr<graphics::MeshBuffer> meshBuffer_;
    size_t instanceCount_;
    graphics::VertexArray instanceVao_;

    void InitInstanceBuffer();
};