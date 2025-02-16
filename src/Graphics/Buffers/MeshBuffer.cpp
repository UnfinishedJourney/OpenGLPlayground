#include "MeshBuffer.h"
#include "Utilities/Logger.h"
#include "Utilities/Utility.h" // For GLCall, if defined
#include <stdexcept>
#include <vector>
#include <span>
#include <glad/glad.h>

namespace graphics {

    MeshBuffer::MeshBuffer(const Mesh& mesh, const MeshLayout& layout)
        : meshLayout_(layout)
    {
        // Validate that required position data exists.
        if (meshLayout_.hasPositions_ && mesh.positions_.empty()) {
            Logger::GetLogger()->error("MeshBuffer: Mesh requires positions, but none were provided.");
            throw std::runtime_error("Mesh requires positions, but none were provided.");
        }

        vertexCount_ = static_cast<GLuint>(mesh.positions_.size());

        // Calculate the total number of float components per vertex.
        size_t totalComponents = 0;
        if (meshLayout_.hasPositions_)  totalComponents += 3; // (x, y, z)
        if (meshLayout_.hasNormals_ && !mesh.normals_.empty()) totalComponents += 3; // (nx, ny, nz)
        for (size_t i = 0; i < meshLayout_.textureTypes_.size(); ++i) {
            if (meshLayout_.textureTypes_.test(i)) {
                totalComponents += 2; // (u, v)
            }
        }
        if (meshLayout_.hasTangents_ && !mesh.tangents_.empty()) totalComponents += 3;

        // Interleave vertex data.
        std::vector<float> vertexData;
        vertexData.reserve(vertexCount_ * totalComponents);

        for (size_t i = 0; i < vertexCount_; ++i) {
            if (meshLayout_.hasPositions_) {
                const auto& pos = mesh.positions_[i];
                vertexData.insert(vertexData.end(), { pos.x, pos.y, pos.z });
            }
            if (meshLayout_.hasNormals_ && !mesh.normals_.empty()) {
                const auto& normal = mesh.normals_[i];
                vertexData.insert(vertexData.end(), { normal.x, normal.y, normal.z });
            }
            for (size_t j = 0; j < meshLayout_.textureTypes_.size(); ++j) {
                if (meshLayout_.textureTypes_.test(j)) {
                    TextureType texType = static_cast<TextureType>(j);
                    const auto& uvIt = mesh.uvs_.at(texType);
                    if (!uvIt.empty() && i < uvIt.size()) {
                        const auto& uv = uvIt[i];
                        vertexData.insert(vertexData.end(), { uv.x, uv.y });
                    }
                    else {
                        vertexData.insert(vertexData.end(), { 0.f, 0.f });
                    }
                }
            }
            if (meshLayout_.hasTangents_ && !mesh.tangents_.empty()) {
                const auto& tangent = mesh.tangents_[i];
                vertexData.insert(vertexData.end(), { tangent.x, tangent.y, tangent.z });
            }
        }

        // Create VAO and VBO.
        VAO_ = std::make_unique<VertexArray>();
        VAO_->Bind();

        std::span<const std::byte> vertexSpan{
            reinterpret_cast<const std::byte*>(vertexData.data()),
            vertexData.size() * sizeof(float)
        };
        VBO_ = std::make_unique<VertexBuffer>(vertexSpan);

        // Build the VertexBufferLayout.
        VertexBufferLayout bufferLayout;
        GLuint attributeIndex = 0;
        if (meshLayout_.hasPositions_) {
            bufferLayout.Push<float>(3, attributeIndex++);
        }
        if (meshLayout_.hasNormals_ && !mesh.normals_.empty()) {
            bufferLayout.Push<float>(3, attributeIndex++);
        }
        for (size_t j = 0; j < meshLayout_.textureTypes_.size(); ++j) {
            if (meshLayout_.textureTypes_.test(j)) {
                bufferLayout.Push<float>(2, attributeIndex++);
            }
        }
        if (meshLayout_.hasTangents_ && !mesh.tangents_.empty()) {
            bufferLayout.Push<float>(3, attributeIndex++);
        }

        VAO_->AddBuffer(*VBO_, bufferLayout);

        // Optionally create an IBO if indices are provided.
        if (!mesh.indices_.empty()) {
            indexCount_ = static_cast<GLuint>(mesh.indices_.size());
            hasIndices_ = true;
            std::vector<GLuint> indicesGL(mesh.indices_.begin(), mesh.indices_.end());
            std::span<const GLuint> indexSpan(indicesGL.data(), indicesGL.size());
            IBO_ = std::make_unique<IndexBuffer>(indexSpan);
        }
        else {
            indexCount_ = vertexCount_;
            hasIndices_ = false;
        }

        VAO_->Unbind();
    }

    void MeshBuffer::Bind() const {
        VAO_->Bind();
        if (hasIndices_ && IBO_) {
            IBO_->Bind();
        }
    }

    void MeshBuffer::Unbind() const {
        if (hasIndices_ && IBO_) {
            IBO_->Unbind();
        }
        VAO_->Unbind();
    }

    void MeshBuffer::Render() const {
        Bind();
        if (hasIndices_) {
            GLCall(glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, nullptr));
        }
        else {
            GLCall(glDrawArrays(GL_TRIANGLES, 0, vertexCount_));
        }
        Unbind();
    }

} // namespace graphics