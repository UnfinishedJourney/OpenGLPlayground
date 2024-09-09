#include "Cube.h"

Cube::Cube(const std::string& path_to_texture) : Mesh()
{
    float vertices[] = {
        // Positions         // Texture Coords
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // Vertex 0
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f, // Vertex 1
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // Vertex 2
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // Vertex 3
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // Vertex 4
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // Vertex 5
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // Vertex 6
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f  // Vertex 7
    };

    unsigned int indices[] = {
        // Front face
        0, 1, 2,
        2, 3, 0,

        // Back face
        4, 5, 6,
        6, 7, 4,

        // Left face
        0, 3, 7,
        7, 4, 0,

        // Right face
        1, 5, 6,
        6, 2, 1,

        // Bottom face
        0, 4, 5,
        5, 1, 0,

        // Top face
        3, 2, 6,
        6, 7, 3
    };

    //m_Texture = std::make_unique<Texture>("../assets/cute_dog.png");
    m_Texture = std::make_unique<Texture>(path_to_texture);

    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    m_VB = std::make_unique<VertexBuffer>(vertices, 5 * 8 * sizeof(float));
    VertexBufferLayout layout;

    layout.Push<float>(3);
    layout.Push<float>(2);
    m_VAO = std::make_unique<VertexArray>();
    m_VAO->AddBuffer(*m_VB, layout);
    m_IB = std::make_unique<IndexBuffer>(indices, 36);

    m_Model = glm::mat4(1.0f);
}

Cube::Cube(std::unique_ptr<Texture> tex)
    : Cube()
{
    m_Texture = std::move(tex);
}
