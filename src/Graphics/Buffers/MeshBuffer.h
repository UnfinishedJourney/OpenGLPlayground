#pragma once

#include <glad/glad.h>
#include <memory>
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"
#include "Graphics/Meshes/Mesh.h"      
#include "Graphics/Meshes/MeshLayout.h"// Describes which attributes to include.

namespace Graphics {

    /**
     * @brief Combines a VAO, VBO, and optional IBO into one renderable mesh buffer.
     *
     * Binds vertex attributes according to the provided MeshLayout.
     */
    class MeshBuffer {
    public:
        /**
         * @brief Constructs a MeshBuffer from a mesh and its layout.
         * @param mesh The geometry (positions, normals, indices, etc.).
         * @param layout The layout describing which attributes to include.
         * @throws std::runtime_error if required mesh data is missing.
         */
        MeshBuffer(const Mesh& mesh, const MeshLayout& layout);

        /**
         * @brief Binds the VAO (and IBO if present).
         */
        void Bind() const;

        /**
         * @brief Unbinds the VAO (and IBO if present).
         */
        void Unbind() const;

        /**
         * @brief Renders the mesh using glDrawElements or glDrawArrays.
         */
        void Render() const;

        [[nodiscard]] GLuint GetIndexCount()  const { return m_IndexCount; }
        [[nodiscard]] GLuint GetVertexCount() const { return m_VertexCount; }
        [[nodiscard]] bool   HasIndices()     const { return m_HasIndices; }

    private:
        bool m_HasIndices = false;
        MeshLayout m_MeshLayout;
        std::unique_ptr<VertexArray> m_VAO;
        std::unique_ptr<VertexBuffer> m_VBO;
        std::unique_ptr<IndexBuffer> m_IBO;
        GLuint m_VertexCount = 0;
        GLuint m_IndexCount = 0;
    };

} // namespace Graphics