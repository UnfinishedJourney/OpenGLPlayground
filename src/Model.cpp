#include <GL/glew.h>

#include "Model.h"
#include "Renderer.h"
#include "imgui.h"
#include <GLFW/glfw3.h>
#include <ext.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include <assimp/version.h>
#include <numeric>

Model::Model(const std::string& path)
	: m_FilePath(path)
{

    //lets load one mesh for now

    const aiScene* scene = aiImportFile(path.c_str(), aiProcess_Triangulate);

    if (!scene || !scene->HasMeshes()) {
        printf("Unable to load file\n");
        exit(255);
    }

    const aiMesh* mesh = scene->mMeshes[0];
    std::vector<glm::vec3> positions;
    for (unsigned int i = 0; i != mesh->mNumFaces; i++)
    {
        const aiFace& face = mesh->mFaces[i];
        const unsigned int idx[3] = { face.mIndices[0], face.mIndices[1], face.mIndices[2] };
        for (int j = 0; j != 3; j++)
        {
            const aiVector3D v = mesh->mVertices[idx[j]];
            positions.push_back(glm::vec3(v.x, v.z, v.y));
        }
    }

    std::vector<glm::vec3> normals;
    for (unsigned int i = 0; i != mesh->mNumFaces; i++)
    {
        const aiFace& face = mesh->mFaces[i];
        const unsigned int idx[3] = { face.mIndices[0], face.mIndices[1], face.mIndices[2] };
        for (int j = 0; j != 3; j++)
        {
            const aiVector3D n = mesh->mNormals[idx[j]];
            normals.push_back(glm::vec3(n.x, n.z, n.y));
        }
    }

    std::vector<glm::vec2> uvs;
    for (unsigned int i = 0; i != mesh->mNumFaces; i++)
    {
        const aiFace& face = mesh->mFaces[i];
        const unsigned int idx[3] = { face.mIndices[0], face.mIndices[1], face.mIndices[2] };
        for (int j = 0; j != 3; j++)
        {
            const aiVector3D t = mesh->mTextureCoords[0][idx[j]];
            uvs.push_back(glm::vec2(t.x, t.y));
        }
    }

    std::vector<unsigned int> indices(positions.size());
    std::iota(indices.begin(), indices.end(), 0);

    aiReleaseImport(scene);

    m_Shader = std::make_unique<Shader>("../shaders/Duck.shader");

    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    std::unique_ptr<VertexBuffer> vb = std::make_unique<VertexBuffer>(nullptr, (2 * sizeof(glm::vec3) + sizeof(glm::vec2)) * positions.size());
    vb->AddSubData(positions.data(), sizeof(glm::vec3) * positions.size());
    vb->AddSubData(normals.data(), sizeof(glm::vec3) * normals.size());
    vb->AddSubData(uvs.data(), sizeof(glm::vec2) * uvs.size());

    //std::unique_ptr<VertexBuffer> vb = std::make_unique<VertexBuffer>(positions.data(), sizeof(glm::vec3) * positions.size());

    VertexBufferLayout layout;
    layout.MakeSubBuffered();
    layout.Push<float>(3, sizeof(positions));
    layout.Push<float>(3, sizeof(normals));
    layout.Push<float>(2, sizeof(uvs));
    std::unique_ptr<VertexArray> vao = std::make_unique<VertexArray>();
    vao->AddBuffer(*vb, layout);
    std::unique_ptr<IndexBuffer> ib = std::make_unique<IndexBuffer>(indices.data(), indices.size());

    std::unique_ptr<Texture> tex = std::make_unique<Texture>("../assets/rubber_duck/textures/Duck_baseColor.png");

    m_Mesh = std::make_unique<Mesh>(std::move(vao), std::move(vb), std::move(ib), std::move(tex));
    //m_Shader->Bind();

}

Model::~Model()
{
}

void Model::Draw()
{
    m_Mesh->Draw(m_Shader);
}

Mesh::Mesh(std::unique_ptr<VertexArray> vao, std::unique_ptr<VertexBuffer> vb, std::unique_ptr<IndexBuffer> ib, std::unique_ptr<Texture> tex)
{
	m_VAO = std::move(vao);
	m_VB = std::move(vb);
	m_IB = std::move(ib);
	m_Texture = std::move(tex);
    m_Model = glm::mat4(1.0f);
}

void Mesh::Draw(const std::unique_ptr<Shader>& shader)
{
	//we should bind shader from the model i guess
	Renderer renderer;
	m_Model = glm::rotate(m_Model, (float)glfwGetTime() * 0.001f, glm::vec3(0.5f, 1.0f, 0.0f));
	glm::mat4 mvp = FrameData::s_Projection * FrameData::s_View * m_Model;
	m_VAO->Bind();
	m_IB->Bind();
	m_Texture->Bind();
	shader->Bind();
	shader->SetUniformMat4f("u_MVP", mvp);
	renderer.Draw(*m_VAO, *m_IB, *shader);
	//m_VAO->Unbind();
	//m_IB->Unbind();
	//m_Texture->Unbind();
}

