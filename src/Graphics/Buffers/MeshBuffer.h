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

namespace graphics {

    /**
     * @brief Combines a VAO, VBO, and (optionally) an IBO into one renderable mesh buffer.
     *
     * The vertex attributes are bound according to the provided MeshLayout.
     */
    class MeshBuffer {
    public:
        /**
         * @brief Constructs a MeshBuffer from a Mesh and its layout.
         * @param mesh The geometry (positions, normals, indices, etc.).
         * @param layout The layout describing which attributes to include.
         * @throws std::runtime_error if required mesh data is missing.
         */
        MeshBuffer(const Mesh& mesh, const MeshLayout& layout);

        /// @brief Binds the VAO (and IBO if present).
        void Bind() const;

        /// @brief Unbinds the VAO (and IBO if present).
        void Unbind() const;

        /// @brief Renders the mesh using glDrawElements or glDrawArrays.
        void Render() const;

        [[nodiscard]] GLuint GetIndexCount()  const { return indexCount_; }
        [[nodiscard]] GLuint GetVertexCount() const { return vertexCount_; }
        [[nodiscard]] bool   HasIndices()     const { return hasIndices_; }

    private:
        bool hasIndices_ = false;
        MeshLayout meshLayout_;
        std::unique_ptr<VertexArray> VAO_;
        std::unique_ptr<VertexBuffer> VBO_;
        std::unique_ptr<IndexBuffer> IBO_;
        GLuint vertexCount_ = 0;
        GLuint indexCount_ = 0;
    };

} // namespace graphics