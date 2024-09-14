#include <GL/glew.h>

#include "Model.h"
#include "Renderer.h"
#include "imgui.h"
#include <GLFW/glfw3.h>
#include <ext.hpp>
#include <numeric>


Model::Model(const std::string& path_to_model)
    : m_FilePath(path_to_model)
{
    ProcessModel();
}

void Model::ProcessModel()
{
    const aiScene* scene = aiImportFile(m_FilePath.c_str(), aiProcess_Triangulate);

    if (!scene || !scene->HasMeshes()) {
        printf("Unable to load file\n");
        exit(255);
    }

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

void Model::ProcessMesh(const aiScene* scene, const aiMesh* aiMesh)
{
    std::shared_ptr<Mesh> myMesh = std::make_shared<Mesh>();

    for (unsigned int i = 0; i < aiMesh->mNumVertices; i++)
    {
        glm::vec3 temp =
        {
            aiMesh->mVertices[i].x,
            aiMesh->mVertices[i].y,
            aiMesh->mVertices[i].z
        };

        myMesh->positions.push_back(temp);

        if (aiMesh->HasNormals())
        {
            temp =
            {
                aiMesh->mNormals[i].x,
                aiMesh->mNormals[i].y,
                aiMesh->mNormals[i].z
            };

            myMesh->normals.push_back(temp);
        }

        if (aiMesh->mTextureCoords[0])
        {
            myMesh->uvs.push_back(glm::vec2(aiMesh->mTextureCoords[0][i].x, aiMesh->mTextureCoords[0][i].y));
        }
    }

    std::vector<unsigned int> indices;

    for (unsigned int i = 0; i < aiMesh->mNumFaces; i++)
    {
        aiFace face = aiMesh->mFaces[i];
        // retrieve all indices of the face and store them in the indices vector
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            myMesh->indices.push_back(face.mIndices[j]);
    }

    m_Meshes.push_back(myMesh);
}


Model::~Model()
{
}


