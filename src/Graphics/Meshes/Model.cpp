#include "Graphics/Meshes/Model.h"
#include "Utilities/Logger.h"
#include <stdexcept>
#include <filesystem>

Model::Model(const std::string& pathToModel, bool centerModel)
    : m_FilePath(pathToModel) {
    ProcessModel();
    m_MeshBuffersCache.resize(m_MeshesInfo.size());

    if (centerModel) {
        CenterModel();
    }
}

void Model::ProcessModel() {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(m_FilePath, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

    if (!scene || !scene->HasMeshes()) {
        throw std::runtime_error("Unable to load model: " + m_FilePath);
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
        // Positions
        myMesh->positions.emplace_back(aiMesh->mVertices[i].x, aiMesh->mVertices[i].y, aiMesh->mVertices[i].z);

        // Normals
        if (aiMesh->HasNormals()) {
            myMesh->normals.emplace_back(aiMesh->mNormals[i].x, aiMesh->mNormals[i].y, aiMesh->mNormals[i].z);
        }

        // Tangents and Bitangents
        if (aiMesh->HasTangentsAndBitangents()) {
            myMesh->tangents.emplace_back(aiMesh->mTangents[i].x, aiMesh->mTangents[i].y, aiMesh->mTangents[i].z);
            myMesh->bitangents.emplace_back(aiMesh->mBitangents[i].x, aiMesh->mBitangents[i].y, aiMesh->mBitangents[i].z);
        }

        // Texture Coordinates
        if (aiMesh->HasTextureCoords(0)) {
            myMesh->uvs[TextureType::Albedo].emplace_back(aiMesh->mTextureCoords[0][i].x, aiMesh->mTextureCoords[0][i].y);
        }
    }

    // Process indices
    for (unsigned int i = 0; i < aiMesh->mNumFaces; i++) {
        aiFace face = aiMesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            myMesh->indices.push_back(face.mIndices[j]);
        }
    }

    // Load textures
    MeshTextures meshTextures;
    std::filesystem::path directory = std::filesystem::path(m_FilePath).parent_path();
    if (aiMesh->mMaterialIndex >= 0) {
        aiMaterial* aiMat = scene->mMaterials[aiMesh->mMaterialIndex];
        meshTextures = LoadTextures(scene, aiMat, directory.string());
    }

    m_MeshesInfo.push_back(MeshInfo{ std::move(meshTextures), std::move(myMesh) });
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

    for (const auto& [aiType, myType] : aiToMyTextureType) {
        unsigned int textureCount = aiMat->GetTextureCount(aiType);
        for (unsigned int i = 0; i < textureCount; ++i) {
            aiString str;
            if (aiMat->GetTexture(aiType, i, &str) == AI_SUCCESS) {
                std::string texturePath = str.C_Str();
                std::filesystem::path fullPath = std::filesystem::path(directory) / texturePath;

                try {
                    auto texture = std::make_shared<Texture2D>(fullPath.string());
                    result.textures[myType] = std::move(texture);
                }
                catch (const std::exception& e) {
                    Logger::GetLogger()->warn("Couldn't load texture '{}': {}", fullPath.string(), e.what());
                }
            }
        }
    }

    return result;
}

glm::vec3 Model::CalculateModelCenter() const {
    glm::vec3 center(0.0f);
    size_t totalVertices = 0;

    for (const auto& meshInfo : m_MeshesInfo) {
        const auto& mesh = meshInfo.mesh;
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
        auto& mesh = meshInfo.mesh;
        for (auto& position : mesh->positions) {
            position -= center;
        }
    }
}

std::shared_ptr<MeshBuffer> Model::GetMeshBuffer(size_t meshIndex, const MeshLayout& layout) {
    if (meshIndex >= m_MeshesInfo.size()) {
        throw std::out_of_range("Invalid mesh index: " + std::to_string(meshIndex));
    }

    auto& cache = m_MeshBuffersCache[meshIndex];
    auto it = cache.find(layout);
    if (it != cache.end()) {
        return it->second;
    }
    else {
        auto meshBuffer = std::make_shared<MeshBuffer>(*m_MeshesInfo[meshIndex].mesh, layout);
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
            auto meshBuffer = std::make_shared<MeshBuffer>(*m_MeshesInfo[i].mesh, layout);
            cache[layout] = meshBuffer;
            result[i] = meshBuffer;
        }
    }

    return result;
}

std::shared_ptr<Texture2D> Model::GetTexture(size_t meshIndex, TextureType type) const {
    if (meshIndex >= m_MeshesInfo.size()) {
        throw std::out_of_range("Invalid mesh index: " + std::to_string(meshIndex));
    }
    auto it = m_MeshesInfo[meshIndex].meshTextures.textures.find(type);
    if (it != m_MeshesInfo[meshIndex].meshTextures.textures.end()) {
        return it->second;
    }
    return nullptr;
}