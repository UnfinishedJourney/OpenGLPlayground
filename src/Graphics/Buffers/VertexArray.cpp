#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexBufferLayout.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h"
#include <algorithm>
#include <stdexcept>

namespace Graphics {

    VertexArray::VertexArray()
    {
        GLCall(glCreateVertexArrays(1, &renderer_id_));
        if (renderer_id_ == 0) {
            Logger::GetLogger()->error("Failed to create Vertex Array Object.");
            throw std::runtime_error("Failed to create Vertex Array Object.");
        }
        Logger::GetLogger()->info("Created VertexArray (ID={}).", renderer_id_);
    }

    VertexArray::~VertexArray()
    {
        if (renderer_id_ != 0) {
            GLCall(glDeleteVertexArrays(1, &renderer_id_));
            Logger::GetLogger()->info("Deleted VertexArray (ID={}).", renderer_id_);
        }
    }

    VertexArray::VertexArray(VertexArray&& other) noexcept
        : renderer_id_{ other.renderer_id_ },
        binding_indices_{ std::move(other.binding_indices_) }
    {
        other.renderer_id_ = 0;
    }

    VertexArray& VertexArray::operator=(VertexArray&& other) noexcept
    {
        if (this != &other) {
            if (renderer_id_ != 0) {
                GLCall(glDeleteVertexArrays(1, &renderer_id_));
            }
            renderer_id_ = other.renderer_id_;
            binding_indices_ = std::move(other.binding_indices_);
            other.renderer_id_ = 0;
        }
        return *this;
    }

    void VertexArray::AddBuffer(const VertexBuffer& vertex_buffer,
        const VertexBufferLayout& layout,
        GLuint binding_index)
    {
        // Prevent using the same binding index more than once.
        if (std::find(binding_indices_.begin(), binding_indices_.end(), binding_index) != binding_indices_.end()) {
            Logger::GetLogger()->error("Binding index={} is already used in VertexArray (ID={}).", binding_index, renderer_id_);
            throw std::invalid_argument("Duplicate binding index in VertexArray::AddBuffer");
        }

        // Attach the buffer at the specified binding index using Direct State Access (DSA).
        GLCall(glVertexArrayVertexBuffer(renderer_id_, binding_index, vertex_buffer.GetRendererID(), 0, layout.GetStride()));

        // Configure each vertex attribute defined in the layout.
        for (const auto& element : layout.GetElements()) {
            GLuint attrib_index = element.attribute_index_;
            GLCall(glEnableVertexArrayAttrib(renderer_id_, attrib_index));
            GLCall(glVertexArrayAttribFormat(renderer_id_, attrib_index, element.count_, element.type_, element.normalized_, element.offset_));
            GLCall(glVertexArrayAttribBinding(renderer_id_, attrib_index, binding_index));
        }

        binding_indices_.push_back(binding_index);
        Logger::GetLogger()->info("Added VertexBuffer (ID={}) to VertexArray (ID={}) at binding index={}.",
            vertex_buffer.GetRendererID(), renderer_id_, binding_index);
    }

    void VertexArray::SetIndexBuffer(const IndexBuffer& index_buffer)
    {
        GLCall(glVertexArrayElementBuffer(renderer_id_, index_buffer.GetRendererID()));
        Logger::GetLogger()->info("Set IndexBuffer (ID={}) to VertexArray (ID={}).", index_buffer.GetRendererID(), renderer_id_);
    }

    void VertexArray::Bind() const
    {
        GLCall(glBindVertexArray(renderer_id_));
    }

    void VertexArray::Unbind() const
    {
        GLCall(glBindVertexArray(0));
    }

} // namespace Graphics