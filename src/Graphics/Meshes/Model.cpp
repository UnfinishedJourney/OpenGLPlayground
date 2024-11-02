#include "Graphics/Meshes/Model.h"
#include "Graphics/Buffers/MeshBuffer.h"
#include "Utilities/Logger.h"

Model::Model(const std::string& path_to_model, bool bCenterModel)
    : m_FilePath(path_to_model) {
    ProcessModel();
    m_MeshBuffersCache.resize(m_MeshesInfo.size());

    if (bCenterModel) {
        CenterModel();
    }
}

void Model::ProcessModel() {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(m_FilePath, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

    if (!scene || !scene->HasMeshes()) {
        Logger::GetLogger()->error("Unable to load file: {}", m_FilePath);
        return;
    }

    ProcessNode(scene, scene->mRootNode);
}

void Model::ProcessNode(const aiScene* scene, const aiNode* node) {
    // Process each mesh located at the current node
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        ProcessMesh(scene, mesh);
    }

    // Recursively process each of the children nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        ProcessNode(scene, node->mChildren[i]);
    }
}

void Model::ProcessMesh(const aiScene* scene, const aiMesh* aiMesh) {
    auto myMesh = std::make_shared<Mesh>();

    myMesh->positions.reserve(aiMesh->mNumVertices);
    if (aiMesh->HasNormals()) {
        myMesh->normals.reserve(aiMesh->mNumVertices);
    }
    if (aiMesh->HasTextureCoords(0)) {
        myMesh->uvs[TextureType::Albedo].reserve(aiMesh->mNumVertices);
    }
    if (aiMesh->HasTangentsAndBitangents()) {
        myMesh->tangents.reserve(aiMesh->mNumVertices);
        myMesh->bitangents.reserve(aiMesh->mNumVertices);
    }

    for (unsigned int i = 0; i < aiMesh->mNumVertices; i++) {
        glm::vec3 temp = {
            aiMesh->mVertices[i].x,
            aiMesh->mVertices[i].y,
            aiMesh->mVertices[i].z
        };
        myMesh->positions.push_back(temp);

        if (aiMesh->HasNormals()) {
            temp = {
                aiMesh->mNormals[i].x,
                aiMesh->mNormals[i].y,
                aiMesh->mNormals[i].z
            };
            myMesh->normals.push_back(temp);
        }

        if (aiMesh->HasTangentsAndBitangents()) {
            glm::vec3 tangent = {
                aiMesh->mTangents[i].x,
                aiMesh->mTangents[i].y,
                aiMesh->mTangents[i].z
            };
            myMesh->tangents.push_back(tangent);

            glm::vec3 bitangent = {
                aiMesh->mBitangents[i].x,
                aiMesh->mBitangents[i].y,
                aiMesh->mBitangents[i].z
            };
            myMesh->bitangents.push_back(bitangent);
        }

        if (aiMesh->HasTextureCoords(0)) {
            myMesh->uvs[TextureType::Albedo].push_back({
                aiMesh->mTextureCoords[0][i].x,
                aiMesh->mTextureCoords[0][i].y
                });
        }
    }

    for (unsigned int i = 0; i < aiMesh->mNumFaces; i++) {
        aiFace face = aiMesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            myMesh->indices.push_back(face.mIndices[j]);
        }
    }

    MeshTextures meshTextures;

    std::string directory = m_FilePath.substr(0, m_FilePath.find_last_of('/'));
    if (aiMesh->mMaterialIndex >= 0) {
        aiMaterial* aiMat = scene->mMaterials[aiMesh->mMaterialIndex];
        meshTextures = LoadTextures(scene, aiMat, directory);
    }

    m_MeshesInfo.push_back(std::make_unique<MeshInfo>(MeshInfo{ meshTextures, myMesh }));
}

MeshTextures Model::LoadTextures(const aiScene* scene, aiMaterial* aiMat, const std::string& directory) {
    MeshTextures result;

    std::unordered_map<aiTextureType, TextureType> aiToMyTextureType = {
        { aiTextureType_DIFFUSE, TextureType::Albedo },
        { aiTextureType_NORMALS, TextureType::Normal },
        { aiTextureType_LIGHTMAP, TextureType::Occlusion },
        { aiTextureType_EMISSIVE, TextureType::Emissive },
        // Add more mappings as needed
    };

    for (const auto& pair : aiToMyTextureType) {
        aiTextureType aiType = pair.first;
        TextureType myType = pair.second;

        unsigned int textureCount = aiMat->GetTextureCount(aiType);
        for (unsigned int i = 0; i < textureCount; ++i) {
            aiString str;
            if (aiMat->GetTexture(aiType, i, &str) == AI_SUCCESS) {
                std::string texturePath = str.C_Str();
                std::string fullPath = directory + "/" + texturePath;

                auto texture = std::make_shared<Texture2D>(fullPath);
                if (texture) {
                    result.textures[myType] = texture;
                }
                else {
                    Logger::GetLogger()->warn("Couldn't load texture: {}", fullPath);
                }
            }
        }
    }

    // Handle other texture types if needed
    // ...

    return result;
}

glm::vec3 Model::CalculateModelCenter() const {
    glm::vec3 center(0.0f);
    size_t totalVertices = 0;

    for (const auto& meshInfo : m_MeshesInfo) {
        auto& mesh = meshInfo->mesh;
        totalVertices += mesh->positions.size();
        for (const auto& position : mesh->positions) {
            center += position;
        }
    }

    if (totalVertices > 0) {
        center /= static_cast<float>(totalVertices);
    }

    return center;
}

void Model::CenterModel() {
    glm::vec3 center = CalculateModelCenter();

    for (auto& meshInfo : m_MeshesInfo) {
        auto& mesh = meshInfo->mesh;
        for (auto& position : mesh->positions) {
            position -= center;
        }
    }
}

std::shared_ptr<MeshBuffer> Model::GetMeshBuffer(size_t meshIndex, const MeshLayout& layout) {
    if (meshIndex >= m_MeshesInfo.size()) {
        Logger::GetLogger()->error("Invalid mesh index: {}", meshIndex);
        return nullptr;
    }

    auto& cache = m_MeshBuffersCache[meshIndex];
    auto it = cache.find(layout);
    if (it != cache.end()) {
        return it->second;
    }
    else {
        auto meshBuffer = std::make_shared<MeshBuffer>(*m_MeshesInfo[meshIndex]->mesh, layout);
        cache[layout] = meshBuffer;
        return meshBuffer;
    }
}

std::vector<std::shared_ptr<MeshBuffer>> Model::GetMeshBuffers(const MeshLayout& layout) {
    std::vector<std::shared_ptr<MeshBuffer>> result(m_MeshesInfo.size());

    for (size_t i = 0; i < m_MeshesInfo.size(); i++) {
        auto& cache = m_MeshBuffersCache[i];
        auto it = cache.find(layout);
        if (it != cache.end()) {
            result[i] = it->second;
        }
        else {
            auto meshBuffer = std::make_shared<MeshBuffer>(*m_MeshesInfo[i]->mesh, layout);
            cache[layout] = meshBuffer;
            result[i] = meshBuffer;
        }
    }

    return result;
}

