#include "Model.h"
#include "Utilities/Logger.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stdexcept>
#include <meshoptimizer.h>
#include <cfloat>
#include <filesystem>

// Helper function to convert aiTextureType to string
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

Model::Model(const std::string& pathToModel, bool centerModel)
    : m_FilePath(pathToModel) {
    LoadModel();
    m_MeshBuffersCache.resize(m_MeshInfos.size());

    if (centerModel) {
        CenterModel();
    }
}

void Model::LoadModel() {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(m_FilePath,
        aiProcess_JoinIdenticalVertices | aiProcess_Triangulate |
        aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace |
        aiProcess_PreTransformVertices | aiProcess_RemoveRedundantMaterials |
        aiProcess_FindInvalidData | aiProcess_OptimizeMeshes);

    if (!scene || !scene->HasMeshes()) {
        Logger::GetLogger()->error("Unable to load model: {}", m_FilePath);
        throw std::runtime_error("Unable to load model: " + m_FilePath);
    }

    Logger::GetLogger()->info("Successfully loaded model: {}", m_FilePath);
    ProcessNode(scene, scene->mRootNode);
}

void Model::ProcessNode(const aiScene* scene, const aiNode* node) {
    Logger::GetLogger()->debug("Processing node: {}", node->mName.C_Str());

    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* aiMesh = scene->mMeshes[node->mMeshes[i]];
        ProcessMesh(scene, aiMesh);
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        ProcessNode(scene, node->mChildren[i]);
    }
}

void Model::ProcessMesh(const aiScene* scene, const aiMesh* aiMesh) {
    Logger::GetLogger()->debug("Processing mesh: {}", aiMesh->mName.C_Str());

    auto myMesh = std::make_shared<Mesh>();

    myMesh->minBounds = glm::vec3(FLT_MAX);
    myMesh->maxBounds = glm::vec3(-FLT_MAX);

    // Process positions
    if (aiMesh->mNumVertices > 0) {
        std::vector<glm::vec3> positionsVec;
        positionsVec.reserve(aiMesh->mNumVertices);
        for (unsigned int i = 0; i < aiMesh->mNumVertices; i++) {
            glm::vec3 pos(aiMesh->mVertices[i].x, aiMesh->mVertices[i].y, aiMesh->mVertices[i].z);
            positionsVec.push_back(pos);
            myMesh->minBounds = glm::min(myMesh->minBounds, pos);
            myMesh->maxBounds = glm::max(myMesh->maxBounds, pos);
        }
        myMesh->positions = std::move(positionsVec);
    }

    // Local center & bounding radius
    myMesh->localCenter = (myMesh->minBounds + myMesh->maxBounds) * 0.5f;
    myMesh->boundingSphereRadius = glm::length(myMesh->maxBounds - myMesh->localCenter);

    // Process normals
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

    // Process UVs (only first UV set)
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

    // Process faces into indices
    std::vector<uint32_t> srcIndices;
    srcIndices.reserve(aiMesh->mNumFaces * 3);
    for (unsigned int i = 0; i < aiMesh->mNumFaces; i++) {
        const aiFace& face = aiMesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            srcIndices.push_back(face.mIndices[j]);
        }
    }

    // Flatten positions to a float array for LOD generation
    std::vector<float> srcVertices;
    //srcVertices.reserve(myMesh->posit * 3);
    std::visit([&](const auto& positionsVec) {
        srcVertices.reserve(positionsVec.size() * 3);
        for (const auto& position : positionsVec) {
            // Ensure always 3D coords
            if constexpr (std::is_same_v<std::decay_t<decltype(position)>, glm::vec3>) {
                srcVertices.push_back(position.x);
                srcVertices.push_back(position.y);
                srcVertices.push_back(position.z);
            }
            else {
                srcVertices.push_back(position.x);
                srcVertices.push_back(position.y);
                srcVertices.push_back(0.0f);
            }
        }
        }, myMesh->positions);

    std::vector<std::vector<uint32_t>> outLods;
    // Generate LODs
    ProcessLODs(srcIndices, srcVertices, outLods);

    // Store LODs in myMesh
    myMesh->indices.clear();
    myMesh->lods.clear();
    for (const auto& lodIndices : outLods) {
        MeshLOD lod;
        lod.indexOffset = static_cast<uint32_t>(myMesh->indices.size());
        lod.indexCount = static_cast<uint32_t>(lodIndices.size());

        myMesh->indices.insert(myMesh->indices.end(), lodIndices.begin(), lodIndices.end());
        myMesh->lods.push_back(lod);
    }

    // Load textures
    MeshTextures meshTextures;
    std::filesystem::path directory = std::filesystem::path(m_FilePath).parent_path();
    if (aiMesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[aiMesh->mMaterialIndex];
        meshTextures = LoadTextures(material, directory.string());
    }

    m_MeshInfos.push_back(MeshInfo{ std::move(meshTextures), std::move(myMesh) });

    //myMesh->meshTextures = std::move(meshTextures);
    //m_MeshInfos.push_back(MeshInfo{ std::move(myMesh->meshTextures), std::move(myMesh) });
}

void Model::ProcessLODs(std::vector<uint32_t>& indices, const std::vector<float>& vertices, std::vector<std::vector<uint32_t>>& outLods) {
    size_t vertexCount = vertices.size() / 3;
    size_t targetIndicesCount = indices.size();
    uint8_t lodLevel = 1;

    Logger::GetLogger()->info("LOD0: {} indices", indices.size());
    outLods.push_back(indices);

    // Try to generate up to 8 LOD levels or until indices count is small
    while (targetIndicesCount > 1024 && lodLevel < 8) {
        targetIndicesCount = indices.size() / 2;
        bool sloppy = false;

        size_t numOptIndices = meshopt_simplify(
            indices.data(), indices.data(), static_cast<uint32_t>(indices.size()),
            vertices.data(), static_cast<size_t>(vertexCount), sizeof(float) * 3,
            static_cast<size_t>(targetIndicesCount), 0.02f);

        // If not much reduction, try sloppy simplification
        if (static_cast<size_t>(numOptIndices * 1.1f) > indices.size()) {
            if (lodLevel > 1) {
                numOptIndices = meshopt_simplifySloppy(
                    indices.data(),
                    indices.data(),
                    static_cast<uint32_t>(indices.size()),
                    vertices.data(),
                    static_cast<size_t>(vertexCount),
                    sizeof(float) * 3,
                    static_cast<size_t>(targetIndicesCount),
                    FLT_MAX,
                    nullptr
                );
                sloppy = true;
                if (numOptIndices == indices.size()) {
                    Logger::GetLogger()->warn("LOD{}: No further simplification possible.", lodLevel);
                    break;
                }
            }
            else {
                Logger::GetLogger()->warn("LOD{}: Simplification did not reduce indices sufficiently.", lodLevel);
                break;
            }
        }

        indices.resize(numOptIndices);
        meshopt_optimizeVertexCache(indices.data(), indices.data(), static_cast<uint32_t>(indices.size()), vertexCount);

        Logger::GetLogger()->info("LOD{}: {} indices {}", lodLevel, numOptIndices, sloppy ? "[sloppy]" : "");
        lodLevel++;

        outLods.push_back(indices);
    }
}

MeshTextures Model::LoadTextures(aiMaterial* material, const std::string& directory)
{
    MeshTextures result;

    // Mapping from aiTextureType to your TextureType enum
    std::unordered_map<aiTextureType, TextureType> aiToMyTextureType = {
        { aiTextureType_DIFFUSE, TextureType::Albedo },
        { aiTextureType_NORMALS, TextureType::Normal },
        { aiTextureType_UNKNOWN, TextureType::MetalRoughness },   // Assuming UNKNOWN is used for MetalRoughness
        { aiTextureType_LIGHTMAP, TextureType::AO },             // Assuming LIGHTMAP is used for Ambient Occlusion
        { aiTextureType_EMISSIVE, TextureType::Emissive }
    };

    // Log available texture types and counts
    Logger::GetLogger()->debug("Texture counts per type:");
    for (const auto& [aiType, myType] : aiToMyTextureType)
    {
        unsigned int count = material->GetTextureCount(aiType);
        Logger::GetLogger()->debug("{}: {}", AiTextureTypeToString(aiType), count);
    }

    // Iterate through the mapping and load textures
    for (const auto& [aiType, myType] : aiToMyTextureType)
    {
        unsigned int textureCount = material->GetTextureCount(aiType);
        for (unsigned int i = 0; i < textureCount; ++i)
        {
            aiString str;
            if (material->GetTexture(aiType, i, &str) == AI_SUCCESS)
            {
                std::string texturePath = std::string(str.C_Str());
                std::filesystem::path fullPath = std::filesystem::path(directory) / texturePath;

                try {
                    auto texture = std::make_shared<Texture2D>(fullPath.string());
                    if (texture)
                    {
                        result.textures[myType] = texture;
                        Logger::GetLogger()->info("Loaded texture [{}]: {}", AiTextureTypeToString(aiType), fullPath.string());
                    }
                    else
                    {
                        Logger::GetLogger()->error("Failed to load texture: {}", fullPath.string());
                    }
                }
                catch (const std::exception& e) {
                    Logger::GetLogger()->error("Exception while loading texture '{}': {}", fullPath.string(), e.what());
                }
            }
            else
            {
                Logger::GetLogger()->warn("Failed to get texture of type {} at index {}", AiTextureTypeToString(aiType), i);
            }
        }
    }

    return result;
}

glm::vec3 Model::CalculateModelCenter() const {
    glm::vec3 center(0.0f);
    size_t totalVertices = 0;

    for (const auto& meshInfo : m_MeshInfos) {
        std::visit([&](const auto& positionsVec) {
            totalVertices += positionsVec.size();
            for (const auto& position : positionsVec) {
                if constexpr (std::is_same_v<std::decay_t<decltype(position)>, glm::vec3>) {
                    // position is glm::vec3
                    center += position;
                }
                else {
                    // position is glm::vec2
                    center += glm::vec3(position.x, position.y, 0.0f);
                }
            }
            }, meshInfo.mesh->positions);
    }

    if (totalVertices > 0) {
        center /= static_cast<float>(totalVertices);
        Logger::GetLogger()->info("Calculated model center: ({}, {}, {})", center.x, center.y, center.z);
    }
    else {
        Logger::GetLogger()->warn("No vertices found to calculate model center.");
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

        mesh->maxBounds -= center;
        mesh->minBounds -= center;
        mesh->localCenter -= center;
    }

    Logger::GetLogger()->info("Model centered by subtracting ({}, {}, {})", center.x, center.y, center.z);
}

std::shared_ptr<MeshBuffer> Model::GetMeshBuffer(size_t meshIndex, const MeshLayout& layout) {
    if (meshIndex >= m_MeshInfos.size()) {
        Logger::GetLogger()->error("Invalid mesh index: {}", meshIndex);
        throw std::out_of_range("Invalid mesh index: " + std::to_string(meshIndex));
    }

    auto& cache = m_MeshBuffersCache[meshIndex];
    auto it = cache.find(layout);
    if (it != cache.end()) {
        return it->second;
    }

    auto meshBuffer = std::make_shared<MeshBuffer>(*m_MeshInfos[meshIndex].mesh, layout);
    cache[layout] = meshBuffer;
    Logger::GetLogger()->debug("Created new MeshBuffer for meshIndex: {}, layout: {}", meshIndex, "");
    return meshBuffer;
}

std::vector<std::shared_ptr<MeshBuffer>> Model::GetMeshBuffers(const MeshLayout& layout) {
    std::vector<std::shared_ptr<MeshBuffer>> result;
    result.reserve(m_MeshInfos.size());

    for (size_t i = 0; i < m_MeshInfos.size(); i++) {
        auto& cache = m_MeshBuffersCache[i];
        auto it = cache.find(layout);
        if (it != cache.end()) {
            result.push_back(it->second);
        }
        else {
            auto meshBuffer = std::make_shared<MeshBuffer>(*m_MeshInfos[i].mesh, layout);
            cache[layout] = meshBuffer;
            result.push_back(meshBuffer);
            Logger::GetLogger()->debug("Created new MeshBuffer for meshIndex: {}, layout: {}", i, "");
        }
    }

    return result;
}

std::shared_ptr<Texture2D> Model::GetTexture(size_t meshIndex, TextureType type) const {
    if (meshIndex >= m_MeshInfos.size()) {
        Logger::GetLogger()->error("Invalid mesh index: {}", meshIndex);
        throw std::out_of_range("Invalid mesh index: " + std::to_string(meshIndex));
    }

    const auto& textures = m_MeshInfos[meshIndex].meshTextures.textures;
    auto it = textures.find(type);
    if (it != textures.end()) {
        return it->second;
    }

    Logger::GetLogger()->warn("Texture of type {} not found for meshIndex: {}", static_cast<int>(type), meshIndex);
    return nullptr;
}