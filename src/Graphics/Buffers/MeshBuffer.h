#pragma once

#include <glad/glad.h>
#include <memory>
#include <vector>
#include <span>
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"
#include "Graphics/Meshes/Mesh.h"
#include "Graphics/Meshes/MeshLayout.h"

//probably need to potentially remove this class
namespace graphics {

    /**
     * @brief Combines a VAO, VBO, and optional IBO into a renderable mesh buffer.
     */
    class MeshBuffer {
    public:
        MeshBuffer(const Mesh& mesh, const MeshLayout& layout);
        void Bind() const;
        void Unbind() const;
        void Render() const;

        [[nodiscard]] GLuint GetIndexCount()  const { return indexCount_; }
        [[nodiscard]] GLuint GetVertexCount() const { return vertexCount_; }
        [[nodiscard]] bool   HasIndices()     const { return hasIndices_; }

    private:
        bool hasIndices_ = false;
        MeshLayout meshLayout_;
        std::unique_ptr<VertexArray> vao_;
        std::unique_ptr<VertexBuffer> vbo_;
        std::unique_ptr<IndexBuffer> ibo_;
        GLuint vertexCount_ = 0;
        GLuint indexCount_ = 0;
    };

} // namespace graphics