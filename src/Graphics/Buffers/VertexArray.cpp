#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexBufferLayout.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <algorithm>
#include <stdexcept>

namespace graphics {

    VertexArray::VertexArray() {
        GLCall(glCreateVertexArrays(1, &rendererId_));
        if (rendererId_ == 0) {
            Logger::GetLogger()->error("Failed to create Vertex Array Object.");
            throw std::runtime_error("Failed to create Vertex Array Object.");
        }
        Logger::GetLogger()->info("Created VertexArray (ID={}).", rendererId_);
    }

    VertexArray::~VertexArray() {
        if (rendererId_ != 0) {
            GLCall(glDeleteVertexArrays(1, &rendererId_));
            Logger::GetLogger()->info("Deleted VertexArray (ID={}).", rendererId_);
        }
    }

    VertexArray::VertexArray(VertexArray&& other) noexcept
        : rendererId_{ other.rendererId_ },
        bindingIndices_{ std::move(other.bindingIndices_) }
    {
        other.rendererId_ = 0;
    }

    VertexArray& VertexArray::operator=(VertexArray&& other) noexcept {
        if (this != &other) {
            if (rendererId_ != 0) {
                GLCall(glDeleteVertexArrays(1, &rendererId_));
            }
            rendererId_ = other.rendererId_;
            bindingIndices_ = std::move(other.bindingIndices_);
            other.rendererId_ = 0;
        }
        return *this;
    }

    void VertexArray::AddBuffer(const VertexBuffer& vertexBuffer,
        const VertexBufferLayout& layout,
        GLuint bindingIndex) {
        if (std::find(bindingIndices_.begin(), bindingIndices_.end(), bindingIndex) != bindingIndices_.end()) {
            Logger::GetLogger()->error("Binding index={} is already used in VertexArray (ID={}).", bindingIndex, rendererId_);
            throw std::invalid_argument("Duplicate binding index in VertexArray::AddBuffer");
        }

        GLCall(glVertexArrayVertexBuffer(rendererId_, bindingIndex, vertexBuffer.GetRendererID(), 0, layout.GetStride()));

        for (const auto& element : layout.GetElements()) {
            GLuint attribIndex = element.attributeIndex_;
            GLCall(glEnableVertexArrayAttrib(rendererId_, attribIndex));
            GLCall(glVertexArrayAttribFormat(rendererId_, attribIndex, element.count_, element.type_, element.normalized_, element.offset_));
            GLCall(glVertexArrayAttribBinding(rendererId_, attribIndex, bindingIndex));
        }

        bindingIndices_.push_back(bindingIndex);
        Logger::GetLogger()->info("Added VertexBuffer (ID={}) to VertexArray (ID={}) at binding index={}.",
            vertexBuffer.GetRendererID(), rendererId_, bindingIndex);
    }

    void VertexArray::SetIndexBuffer(const IndexBuffer& indexBuffer) {
        GLCall(glVertexArrayElementBuffer(rendererId_, indexBuffer.GetRendererID()));
        Logger::GetLogger()->info("Set IndexBuffer (ID={}) to VertexArray (ID={}).", indexBuffer.GetRendererID(), rendererId_);
    }

    void VertexArray::Bind() const {
        GLCall(glBindVertexArray(rendererId_));
    }

    void VertexArray::Unbind() const {
        GLCall(glBindVertexArray(0));
    }

} // namespace graphics