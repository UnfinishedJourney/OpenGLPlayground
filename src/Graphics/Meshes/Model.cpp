#include "Model.h"
#include "Utilities/Logger.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <stdexcept>

#define USING_EASY_PROFILER
#include <easy/profiler.h>

Model::Model(const std::string& pathToModel, bool centerModel)
    : m_FilePath(pathToModel) {
    LoadModel();
    m_MeshBuffersCache.resize(m_MeshInfos.size());

    if (centerModel) {
        CenterModel();
    }
}

void Model::LoadModel() {
    EASY_FUNCTION(profiler::colors::Amber50);
    Assimp::Importer importer;
    EASY_BLOCK("Read Scene");
    //slow, need to think about whether or not i can optimize it
    //aiProcess_FindDegenerates bug       aiProcess_FindInstances need to understand
    const aiScene* scene = importer.ReadFile(m_FilePath, aiProcess_JoinIdenticalVertices | aiProcess_Triangulate |
        aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_PreTransformVertices | aiProcess_RemoveRedundantMaterials | aiProcess_FindInvalidData | aiProcess_OptimizeMeshes);

    EASY_END_BLOCK;

    if (!scene || !scene->HasMeshes()) {
        throw std::runtime_error("Unable to load model: " + m_FilePath);
    }

    ProcessNode(scene, scene->mRootNode);
}

void Model::ProcessNode(const aiScene* scene, const aiNode* node) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        EASY_BLOCK("Process Mesh");
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        ProcessMesh(scene, mesh);
        EASY_END_BLOCK;
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        ProcessNode(scene, node->mChildren[i]);
    }
}

void Model::ProcessMesh(const aiScene* scene, const aiMesh* aiMesh) {
    auto myMesh = std::make_shared<Mesh>();

    if (aiMesh->mNumVertices > 0) {
        // Determine if the mesh is 2D or 3D based on positions
        bool is2D = true;
        for (unsigned int i = 0; i < aiMesh->mNumVertices; ++i) {
            if (aiMesh->mVertices[i].z != 0.0f) {
                is2D = false;
                break;
            }
        }

        if (is2D) {
            // Initialize positions as glm::vec2
            std::vector<glm::vec2> positionsVec;
            positionsVec.reserve(aiMesh->mNumVertices);
            for (unsigned int i = 0; i < aiMesh->mNumVertices; i++) {
                positionsVec.emplace_back(
                    aiMesh->mVertices[i].x,
                    aiMesh->mVertices[i].y
                );
            }
            myMesh->positions = std::move(positionsVec);
        }
        else {
            // Initialize positions as glm::vec3
            std::vector<glm::vec3> positionsVec;
            positionsVec.reserve(aiMesh->mNumVertices);
            for (unsigned int i = 0; i < aiMesh->mNumVertices; i++) {
                positionsVec.emplace_back(
                    aiMesh->mVertices[i].x,
                    aiMesh->mVertices[i].y,
                    aiMesh->mVertices[i].z
                );
            }
            myMesh->positions = std::move(positionsVec);
        }
    }

    if (aiMesh->HasNormals()) {
        myMesh->normals.reserve(aiMesh->mNumVertices);
        for (unsigned int i = 0; i < aiMesh->mNumVertices; i++) {
            myMesh->normals.emplace_back(
                aiMesh->mNormals[i].x,
                aiMesh->mNormals[i].y,
                aiMesh->mNormals[i].z
            );
        }
    }

    // Process UVs (only first set)
    if (aiMesh->HasTextureCoords(0)) {
        myMesh->uvs[TextureType::Albedo].reserve(aiMesh->mNumVertices);
        for (unsigned int i = 0; i < aiMesh->mNumVertices; i++) {
            myMesh->uvs[TextureType::Albedo].emplace_back(
                aiMesh->mTextureCoords[0][i].x,
                aiMesh->mTextureCoords[0][i].y
            );
        }
    }

    // Process tangents and bitangents
    if (aiMesh->HasTangentsAndBitangents()) {
        myMesh->tangents.reserve(aiMesh->mNumVertices);
        myMesh->bitangents.reserve(aiMesh->mNumVertices);
        for (unsigned int i = 0; i < aiMesh->mNumVertices; i++) {
            myMesh->tangents.emplace_back(
                aiMesh->mTangents[i].x,
                aiMesh->mTangents[i].y,
                aiMesh->mTangents[i].z
            );
            myMesh->bitangents.emplace_back(
                aiMesh->mBitangents[i].x,
                aiMesh->mBitangents[i].y,
                aiMesh->mBitangents[i].z
            );
        }
    }

    myMesh->indices.reserve(aiMesh->mNumFaces * 3);
    for (unsigned int i = 0; i < aiMesh->mNumFaces; i++) {
        aiFace face = aiMesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            myMesh->indices.push_back(face.mIndices[j]);
        }
    }

    if (myMesh->lods.empty()) {
        MeshLOD defaultLOD;
        defaultLOD.indexOffset = 0;
        defaultLOD.indexCount = static_cast<uint32_t>(myMesh->indices.size());
        myMesh->lods.push_back(defaultLOD);
    }

    MeshTextures meshTextures;
    std::filesystem::path directory = std::filesystem::path(m_FilePath).parent_path();
    if (aiMesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[aiMesh->mMaterialIndex];
        meshTextures = LoadTextures(material, directory.string());
    }

    m_MeshInfos.emplace_back(MeshInfo{ std::move(meshTextures), std::move(myMesh) });
}

MeshTextures Model::LoadTextures(aiMaterial* material, const std::string& directory) {
    MeshTextures result;

    // Mapping from Assimp texture types to your TextureType
    std::unordered_map<aiTextureType, TextureType> aiToMyTextureType = {
        { aiTextureType_DIFFUSE, TextureType::Albedo },
        { aiTextureType_NORMALS, TextureType::Normal },
        { aiTextureType_HEIGHT, TextureType::Occlusion }, // Typically used for height maps or occlusion
        { aiTextureType_EMISSIVE, TextureType::Emissive },
        // Add more mappings as needed
    };

    for (const auto& [aiType, myType] : aiToMyTextureType) {
        unsigned int textureCount = material->GetTextureCount(aiType);
        for (unsigned int i = 0; i < textureCount; ++i) {
            aiString str;
            if (material->GetTexture(aiType, i, &str) == AI_SUCCESS) {
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

    for (const auto& meshInfo : m_MeshInfos) {
        const auto& mesh = meshInfo.mesh;

        std::visit([&](const auto& positionsVec) {
            totalVertices += positionsVec.size();
            for (const auto& position : positionsVec) {
                if constexpr (std::is_same_v<std::decay_t<decltype(position)>, glm::vec3>) {
                    center += position;
                }
                else {
                    center += glm::vec3(position, 0.0f); // Assuming z = 0 for 2D
                }
            }
            }, mesh->positions);
    }

    if (totalVertices > 0) {
        center /= static_cast<float>(totalVertices);
    }

    return center;
}

void Model::CenterModel() {
    glm::vec3 center = CalculateModelCenter();

    for (auto& meshInfo : m_MeshInfos) {
        auto& mesh = meshInfo.mesh;

        std::visit([&](auto& positionsVec) {
            for (auto& position : positionsVec) {
                if constexpr (std::is_same_v<std::decay_t<decltype(position)>, glm::vec3>) {
                    position -= center;
                }
                else {
                    position -= glm::vec2(center.x, center.y);
                }
            }
            }, mesh->positions);
    }
}

std::shared_ptr<MeshBuffer> Model::GetMeshBuffer(size_t meshIndex, const MeshLayout& layout) {
    if (meshIndex >= m_MeshInfos.size()) {
        throw std::out_of_range("Invalid mesh index: " + std::to_string(meshIndex));
    }

    auto& cache = m_MeshBuffersCache[meshIndex];
    auto it = cache.find(layout);
    if (it != cache.end()) {
        return it->second;
    }
    else {
        auto meshBuffer = std::make_shared<MeshBuffer>(*m_MeshInfos[meshIndex].mesh, layout);
        cache[layout] = meshBuffer;
        return meshBuffer;
    }
}

std::vector<std::shared_ptr<MeshBuffer>> Model::GetMeshBuffers(const MeshLayout& layout) {
    std::vector<std::shared_ptr<MeshBuffer>> result(m_MeshInfos.size());

    for (size_t i = 0; i < m_MeshInfos.size(); i++) {
        auto& cache = m_MeshBuffersCache[i];
        auto it = cache.find(layout);
        if (it != cache.end()) {
            result[i] = it->second;
        }
        else {
            auto meshBuffer = std::make_shared<MeshBuffer>(*m_MeshInfos[i].mesh, layout);
            cache[layout] = meshBuffer;
            result[i] = meshBuffer;
        }
    }

    return result;
}

std::shared_ptr<Texture2D> Model::GetTexture(size_t meshIndex, TextureType type) const {
    if (meshIndex >= m_MeshInfos.size()) {
        throw std::out_of_range("Invalid mesh index: " + std::to_string(meshIndex));
    }
    auto it = m_MeshInfos[meshIndex].meshTextures.textures.find(type);
    if (it != m_MeshInfos[meshIndex].meshTextures.textures.end()) {
        return it->second;
    }
    return nullptr;
}