#include <glad/glad.h>

#include "Graphics/Meshes/Model.h"
#include "Renderer/Renderer.h"
#include "imgui.h"
#include <GLFW/glfw3.h>
#include <ext.hpp>
#include <numeric>


Model::Model(const std::string& path_to_model, bool bCenterModel)
    : m_FilePath(path_to_model)
{
    ProcessModel();
    m_MeshBuffersCache.resize(m_MeshesInfo.size());
    
    if (bCenterModel)
    {
        CenterModel();
    }
}

void Model::ProcessModel()
{
    const aiScene* scene = aiImportFile(m_FilePath.c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals);

    if (!scene || !scene->HasMeshes()) 
    {
        std::cerr << "Unable to load file\n";
        exit(255);
    }

    ProcessNode(scene, scene->mRootNode);

    //LoadTextures();
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

    myMesh->positions.reserve(aiMesh->mNumVertices);
    if (aiMesh->HasNormals())
    {
        myMesh->normals.reserve(aiMesh->mNumVertices);
    }
    if (aiMesh->HasTextureCoords(0))
    {
        myMesh->uvs.reserve(aiMesh->mNumVertices);
    }

    for (unsigned int i = 0; i < aiMesh->mNumVertices; i++)
    {
        glm::vec3 temp =
        {
            aiMesh->mVertices[i].x,
            -aiMesh->mVertices[i].z,
            aiMesh->mVertices[i].y
        };

        myMesh->positions.push_back(temp);

        if (aiMesh->HasNormals())
        {
            temp =
            {
                aiMesh->mNormals[i].x,
                -aiMesh->mVertices[i].z,
                aiMesh->mNormals[i].y
            };

            myMesh->normals.push_back(temp);
        }

        if (aiMesh->HasTextureCoords(0))
        {
            myMesh->uvs.push_back(glm::vec2(aiMesh->mTextureCoords[0][i].x, aiMesh->mTextureCoords[0][i].y));
        }
    }

    for (unsigned int i = 0; i < aiMesh->mNumFaces; i++)
    {
        aiFace face = aiMesh->mFaces[i];
        // retrieve all indices of the face and store them in the indices vector
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            myMesh->indices.push_back(face.mIndices[j]);
    }

    MeshTextures meshTextures;

    std::string directory = m_FilePath.substr(0, m_FilePath.find_last_of('/'));
    if (aiMesh->mMaterialIndex >= 0) {
        aiMaterial* aiMat = scene->mMaterials[aiMesh->mMaterialIndex];
        meshTextures = LoadTextures(scene, aiMat, directory);
    }


    m_MeshesInfo.push_back(std::make_unique<MeshInfo>(MeshInfo(meshTextures, myMesh)));
}

inline std::string AiTextureTypeToString(aiTextureType type)
{
    switch (type)
    {
    case aiTextureType_NONE:
        return "NONE";
    case aiTextureType_DIFFUSE:
        return "DIFFUSE";
    case aiTextureType_SPECULAR:
        return "SPECULAR";
    case aiTextureType_AMBIENT:
        return "AMBIENT";
    case aiTextureType_EMISSIVE:
        return "EMISSIVE";
    case aiTextureType_HEIGHT:
        return "HEIGHT";
    case aiTextureType_NORMALS:
        return "NORMALS";
    case aiTextureType_SHININESS:
        return "SHININESS";
    case aiTextureType_OPACITY:
        return "OPACITY";
    case aiTextureType_DISPLACEMENT:
        return "DISPLACEMENT";
    case aiTextureType_LIGHTMAP:
        return "LIGHTMAP";
    case aiTextureType_REFLECTION:
        return "REFLECTION";
    case aiTextureType_UNKNOWN:
        return "UNKNOWN";
    default:
        return "UNKNOWN";
    }
}

MeshTextures Model::LoadTextures(const aiScene* scene, aiMaterial* aiMat, const std::string& directory)
{
    MeshTextures result;

    std::unordered_map<aiTextureType, TextureType> aiToMyTextureType = {
        {aiTextureType_DIFFUSE, TextureType::Albedo},
        {aiTextureType_NORMALS, TextureType::Normal},
        {aiTextureType_LIGHTMAP, TextureType::Occlusion},
        {aiTextureType_EMISSIVE, TextureType::Emissive},
    };

    for (int i = aiTextureType_NONE; i < 13; ++i)
    {
        aiTextureType type = static_cast<aiTextureType>(i);
        std::cout << AiTextureTypeToString(type) << " " <<  aiMat->GetTextureCount(type) << std::endl;
    }

    for (const auto& pair : aiToMyTextureType)
    {
        aiTextureType aiType = pair.first;
        TextureType myType = pair.second;

        unsigned int textureCount = aiMat->GetTextureCount(aiType);
        for (unsigned int i = 0; i < textureCount; ++i)
        {
            aiString str;
            if (aiMat->GetTexture(aiType, i, &str) == AI_SUCCESS)
            {
                std::string texturePath = std::string(str.C_Str());
                std::string fullPath = directory + "/" + texturePath;

                std::shared_ptr<Texture2D> texture = std::make_shared<Texture2D>(fullPath);
                if (texture)
                {
                    result.textures[myType] = texture;
                }
                else
                {
                    std::cerr << "Couldn't load texture: " << fullPath << std::endl;
                }
            }
        }
    }

    unsigned int specularCount = aiMat->GetTextureCount(aiTextureType_UNKNOWN);
    for (unsigned int i = 0; i < specularCount; ++i)
    {
        aiString str;
        if (aiMat->GetTexture(aiTextureType_UNKNOWN, i, &str) == AI_SUCCESS)
        {
            std::string texturePath = std::string(str.C_Str());
            std::string fullPath = directory + "/" + texturePath;

            std::shared_ptr<Texture2D> texture = std::make_shared<Texture2D>(fullPath);
            if (texture)
            {
                result.textures[TextureType::RoughnessMetallic] = texture;
            }
            else
            {
                std::cerr << "Couldn't load RoughnessMetallic: " << fullPath << std::endl;
            }
        }
    }

    return result;
}

glm::vec3 Model::CalculateModelCenter() const
{
    glm::vec3 center(0.0f);
    size_t totalVertices = 0;

    for (const auto& meshInfo : m_MeshesInfo)
    {
        auto& mesh = meshInfo->mesh;
        totalVertices += mesh->positions.size();
        for (const auto& position : mesh->positions)
        {
            center += position;
        }
    }

    if (totalVertices > 0)
    {
        center /= static_cast<float>(totalVertices);
    }

    return center;
}

void Model::CenterModel()
{
    glm::vec3 center = CalculateModelCenter();

    for (auto& meshInfo : m_MeshesInfo)
    {
        auto& mesh = meshInfo->mesh;
        for (auto& position : mesh->positions)
        {
            position -= center;
        }
    }
}

std::shared_ptr<MeshBuffer> Model::GetMeshBuffer(size_t meshIndex, const MeshLayout& layout)
{
    if (meshIndex >= m_MeshesInfo.size())
    {
        std::cerr << "Invalid mesh index: " << meshIndex << "\n";
        return nullptr;
    }

    auto& cache = m_MeshBuffersCache[meshIndex];
    auto it = cache.find(layout);
    if (it != cache.end())
    {
        return it->second;
    }
    else
    {
        auto meshBuffer = std::make_shared<MeshBuffer>(m_MeshesInfo[meshIndex]->mesh, layout);
        cache[layout] = meshBuffer;
        return meshBuffer;
    }
}

std::vector<std::shared_ptr<MeshBuffer>> Model::GetMeshBuffers(const MeshLayout& layout)
{
    std::vector<std::shared_ptr<MeshBuffer>> result(m_MeshesInfo.size());

    for (size_t i = 0; i < m_MeshesInfo.size(); i++)
    {
        auto& cache = m_MeshBuffersCache[i];
        auto it = cache.find(layout);
        if (it != cache.end())
        {
            result[i] = it->second;
        }
        else
        {
            auto meshBuffer = std::make_shared<MeshBuffer>(m_MeshesInfo[i]->mesh, layout);
            cache[layout] = meshBuffer;
            result[i] = meshBuffer;
        }
    }

    return result;
}

