#include <GL/glew.h>

#include "Model.h"
#include "Renderer.h"
#include "imgui.h"
#include <GLFW/glfw3.h>
#include <ext.hpp>
#include <numeric>

namespace Model {

    Model::Model(const std::string& path_to_model, const std::string& path_to_shader)
        : m_FilePath(path_to_model)
    {
        ProcessModel();
        m_Shader = std::make_unique<Shader>(path_to_shader);
    }

    void Model::ProcessModel()
    {
        const aiScene* scene = aiImportFile(m_FilePath.c_str(), aiProcess_Triangulate);

        if (!scene || !scene->HasMeshes()) {
            printf("Unable to load file\n");
            exit(255);
        }

        //ProcessMesh(scene, scene->mMeshes[0]);
        ProcessNode(scene, scene->mRootNode);
        aiReleaseImport(scene);
    }

    void Model::ProcessNode(const aiScene* scene, const aiNode* node)
    {
        // process each mesh located at the current node
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // the node object only contains indices to index the actual objects in the scene. 
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            ProcessMesh(scene, mesh);
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            ProcessNode(scene, node->mChildren[i]);
        }

    }

    /*void Model::ProcessMesh(const aiScene* scene, const aiMesh* mesh)
    {
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> uvs;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            positions.push_back(glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z));

            if (mesh->HasNormals())
            {
                normals.push_back(glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z));
            }

            if (mesh->mTextureCoords[0])
            {
                uvs.push_back(glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y));
            }
            else
                uvs.push_back(glm::vec2(0.0f, 0.0f));
        }

        std::vector<unsigned int> indices;

        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        m_Shader = std::make_unique<Shader>("../shaders/Duck.shader");

        GLCall(glEnable(GL_BLEND));
        GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        std::unique_ptr<VertexBuffer> vb = std::make_unique<VertexBuffer>(nullptr, (2 * sizeof(glm::vec3) + sizeof(glm::vec2)) * positions.size());
        vb->AddSubData(&positions[0], sizeof(glm::vec3) * positions.size());
        vb->AddSubData(&normals[0], sizeof(glm::vec3) * normals.size());
        vb->AddSubData(&uvs[0], sizeof(glm::vec2) * uvs.size());

        VertexBufferLayout layout;
        layout.MakeSubBuffered();
        layout.Push<float>(3, sizeof(positions));
        layout.Push<float>(3, sizeof(normals));
        layout.Push<float>(2, sizeof(uvs));
        std::unique_ptr<VertexArray> vao = std::make_unique<VertexArray>();
        vao->AddBuffer(*vb, layout);
        std::unique_ptr<IndexBuffer> ib = std::make_unique<IndexBuffer>(indices.data(), indices.size());

        std::unique_ptr<Texture> tex = std::make_unique<Texture>("../assets/rubber_duck/textures/Duck_baseColor.png");

        //m_Mesh = std::make_unique<Mesh>(std::move(vao), std::move(vb), std::move(ib), std::move(tex));
        m_Meshes.push_back({ std::move(vao), std::move(vb), std::move(ib), std::move(tex) });
    }*/

    void Model::ProcessMesh(const aiScene* scene, const aiMesh* mesh)
    {
        std::vector<Vertex> vertices;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

            if (mesh->HasNormals())
            {
                vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
            }

            if (mesh->mTextureCoords[0])
            {
                vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }

        std::vector<unsigned int> indices;

        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        GLCall(glEnable(GL_BLEND));
        GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        std::unique_ptr<VertexBuffer> vb = std::make_unique<VertexBuffer>(&vertices[0], sizeof(Vertex) * vertices.size());

        VertexBufferLayout layout;
        layout.Push<float>(3);
        layout.Push<float>(3);
        layout.Push<float>(2);
        //layout.SetStride(sizeof(Vertex));
        std::unique_ptr<VertexArray> vao = std::make_unique<VertexArray>();
        vao->AddBuffer(*vb, layout);
        std::unique_ptr<IndexBuffer> ib = std::make_unique<IndexBuffer>(indices.data(), indices.size());

        std::unique_ptr<Texture> tex = std::make_unique<Texture>("../assets/rubber_duck/textures/Duck_baseColor.png");

        //m_Mesh = std::make_unique<Mesh>(std::move(vao), std::move(vb), std::move(ib), std::move(tex));
        m_Meshes.push_back({ std::move(vao), std::move(vb), std::move(ib), std::move(tex) });
    }


    Model::~Model()
    {
    }

    void Model::Draw()
    {
        //m_Mesh->Draw(m_Shader);
        for (auto& m : m_Meshes) {
            m.Draw(m_Shader);
        }
    }

    void Model::Update(float dt)
    {
        for (auto& m : m_Meshes) {
            m.Update(dt);
        }
    }

    Mesh::Mesh(std::unique_ptr<VertexArray> vao, std::unique_ptr<VertexBuffer> vb, std::unique_ptr<IndexBuffer> ib, std::unique_ptr<Texture> tex)
    {
        m_VAO = std::move(vao);
        m_VB = std::move(vb);
        m_IB = std::move(ib);
        m_Texture = std::move(tex);
        m_Model = glm::mat4(1.0f);
        m_Model = glm::rotate(m_Model, -3.14f / 2, glm::vec3(1, 0, 0));
    }

    void Mesh::Draw(const std::unique_ptr<Shader>& shader)
    {
        //we should bind shader from the model i guess
        Renderer renderer;
        glm::mat4 mvp = FrameData::s_Projection * FrameData::s_View * m_Model;
        m_VAO->Bind();
        shader->Bind();
        m_IB->Bind();
        m_Texture->Bind();
        shader->SetUniformMat4f("u_MVP", mvp);
        renderer.Draw(*m_VAO, *m_IB, *shader);
        m_VAO->Unbind();
        m_IB->Unbind();
        //m_Texture->Unbind();
    }

    void Mesh::Draw(const std::shared_ptr<Shader>& shader)
    {
        //we should bind shader from the model i guess
        Renderer renderer;
        glm::mat4 mvp = FrameData::s_Projection * FrameData::s_View * m_Model;
        m_VAO->Bind();
        shader->Bind();
        m_IB->Bind();
        m_Texture->Bind();
        shader->SetUniformMat4f("u_MVP", mvp);
        renderer.Draw(*m_VAO, *m_IB, *shader);
        m_VAO->Unbind();
        m_IB->Unbind();
        //m_Texture->Unbind();
    }


    void Mesh::Update(float dt)
    {
        m_Model = glm::rotate(m_Model, dt * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));
        //m_LastTime = (float)glfwGetTime();
    }


}
