#include "Graphics/Meshes/SkyBox.h"

#include <glad/glad.h>
#include <vector>

SkyBox::SkyBox(GLuint texID, float size)
    : m_TexID(texID), m_Size(size)
{
    float side2 = size * 0.5f;
    std::vector<GLfloat> v = {
        // Front
       -side2, -side2, side2,
        side2, -side2, side2,
        side2,  side2, side2,
       -side2,  side2, side2,
       // Right
        side2, -side2, side2,
        side2, -side2, -side2,
        side2,  side2, -side2,
        side2,  side2, side2,
        // Back
        -side2, -side2, -side2,
        -side2,  side2, -side2,
         side2,  side2, -side2,
         side2, -side2, -side2,
         // Left
         -side2, -side2, side2,
         -side2,  side2, side2,
         -side2,  side2, -side2,
         -side2, -side2, -side2,
         // Bottom
         -side2, -side2, side2,
         -side2, -side2, -side2,
          side2, -side2, -side2,
          side2, -side2, side2,
          // Top
          -side2,  side2, side2,
           side2,  side2, side2,
           side2,  side2, -side2,
          -side2,  side2, -side2
    };

    // We don't shade a sky box, so normals aren't used.
    std::vector<GLfloat> n(v.size(), 0.0f);

    std::vector<GLuint> el = {
        0,2,1,0,3,2,
        4,6,5,4,7,6,
        8,10,9,8,11,10,
        12,14,13,12,15,14,
        16,18,17,16,19,18,
        20,22,21,20,23,22
    };

    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    m_VB = std::make_unique<VertexBuffer>(&v[0], sizeof(GLfloat) * v.size());

    VertexBufferLayout layout;
    layout.Push<float>(3);
    m_VAO = std::make_unique<VertexArray>();
    m_VAO->AddBuffer(*m_VB, layout);
    m_IB = std::make_unique<IndexBuffer>(el.data(), el.size());
}

void SkyBox::Draw(const std::unique_ptr<Shader>& shader) const
{
    Renderer renderer;
    glm::mat4 mvp = FrameData::s_Projection * FrameData::s_View * glm::mat4(1.0);
    shader->Bind();
    m_VAO->Bind();
    m_IB->Bind();
    GLCall(glActiveTexture(GL_TEXTURE0));
    GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, m_TexID));
    shader->SetUniform("u_MVP", mvp);
    renderer.Draw(*m_VAO, *m_IB, *shader);
    m_VAO->Unbind();
    m_IB->Unbind();
}
