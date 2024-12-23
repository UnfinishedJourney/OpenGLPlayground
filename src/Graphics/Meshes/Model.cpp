#include "Model.h"
#include "Utilities/Logger.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <meshoptimizer.h>
#include <stdexcept>
#include <cfloat>
#include <filesystem>

// Helper to convert aiMatrix4x4 to glm::mat4
static glm::mat4 AiToGlm(const aiMatrix4x4& m) {
    glm::mat4 r;
    r[0][0] = m.a1; r[1][0] = m.b1; r[2][0] = m.c1; r[3][0] = m.d1;
    r[0][1] = m.a2; r[1][1] = m.b2; r[2][1] = m.c2; r[3][1] = m.d2;
    r[0][2] = m.a3; r[1][2] = m.b3; r[2][2] = m.c3; r[3][2] = m.d3;
    r[0][3] = m.a4; r[1][3] = m.b4; r[2][3] = m.c4; r[3][3] = m.d4;
    return glm::transpose(r);
}

Model::Model(const std::string& pathToModel, bool centerModel, const MeshLayout& requestedLayout)
    : m_FilePath(pathToModel) {
    LoadFromAssimp(requestedLayout, centerModel);
}

void Model::LoadFromAssimp(const MeshLayout& layout, bool centerModel) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(m_FilePath,
        aiProcess_JoinIdenticalVertices | aiProcess_Triangulate |
        aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace |
        aiProcess_RemoveRedundantMaterials | aiProcess_FindInvalidData |
        aiProcess_OptimizeMeshes);

    if (!scene || !scene->HasMeshes()) {
        Logger::GetLogger()->error("Unable to load model: {}", m_FilePath);
        throw std::runtime_error("Unable to load model: " + m_FilePath);
    }

    // Materials
    m_Materials.resize(scene->mNumMaterials);
    for (unsigned i = 0; i < scene->mNumMaterials; i++) {
        aiString name;
        scene->mMaterials[i]->Get(AI_MATKEY_NAME, name);
        m_Materials[i].name = name.C_Str();
        aiColor3D diffuseColor;
        if (AI_SUCCESS == scene->mMaterials[i]->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor)) {
            diffuseColor;
        }

    }

    // Meshes
    for (unsigned i = 0; i < scene->mNumMeshes; i++) {
        aiMesh* aimesh = scene->mMeshes[i];
        ProcessMesh(scene, aimesh, layout);
    }

    m_Nodes.reserve(64);
    ProcessNode(scene, scene->mRootNode, -1);

    if (centerModel) {
        // TODO: implement center model if needed
    }
}

int Model::AddNode(const std::string& name, int parent, const glm::mat4& localTransform) {
    Node node;
    node.name = name;
    node.parent = parent;
    node.localTransform = localTransform;
    int index = (int)m_Nodes.size();
    m_Nodes.push_back(node);
    if (parent >= 0) {
        m_Nodes[parent].children.push_back(index);
    }
    return index;
}

void Model::ProcessNode(const aiScene* scene, const aiNode* ainode, int parentIndex) {
    glm::mat4 local = AiToGlm(ainode->mTransformation);
    int currentIndex = AddNode(ainode->mName.C_Str(), parentIndex, local);

    for (unsigned i = 0; i < ainode->mNumMeshes; i++) {
        m_Nodes[currentIndex].meshes.push_back((int)ainode->mMeshes[i]);
    }

    for (unsigned c = 0; c < ainode->mNumChildren; c++) {
        ProcessNode(scene, ainode->mChildren[c], currentIndex);
    }
}

void Model::ProcessMesh(const aiScene* scene, const aiMesh* aiMesh, const MeshLayout& layout) {
    auto myMesh = std::make_shared<Mesh>();

    // Positions
    if (layout.hasPositions) {
        std::vector<glm::vec3> positionsVec(aiMesh->mNumVertices);
        for (unsigned v = 0; v < aiMesh->mNumVertices; v++) {
            glm::vec3 pos(aiMesh->mVertices[v].x, aiMesh->mVertices[v].y, aiMesh->mVertices[v].z);
            positionsVec[v] = pos;
            myMesh->minBounds = glm::min(myMesh->minBounds, pos);
            myMesh->maxBounds = glm::max(myMesh->maxBounds, pos);
        }
        myMesh->positions = positionsVec;
    }

    // Normals
    if (layout.hasNormals && aiMesh->HasNormals()) {
        myMesh->normals.reserve(aiMesh->mNumVertices);
        for (unsigned v = 0; v < aiMesh->mNumVertices; v++) {
            myMesh->normals.emplace_back(aiMesh->mNormals[v].x, aiMesh->mNormals[v].y, aiMesh->mNormals[v].z);
        }
    }

    // UVs (only one set for simplicity)
    if (!layout.textureTypes.empty() && aiMesh->HasTextureCoords(0)) {
        std::vector<glm::vec2> uvsVec(aiMesh->mNumVertices);
        for (unsigned v = 0; v < aiMesh->mNumVertices; v++) {
            uvsVec[v] = glm::vec2(aiMesh->mTextureCoords[0][v].x, aiMesh->mTextureCoords[0][v].y);
        }
        // Assign to all requested textureTypes that need uvs:
        for (auto t : layout.textureTypes) {
            myMesh->uvs[t] = uvsVec;
        }
    }

    // Faces to indices
    std::vector<uint32_t> srcIndices;
    srcIndices.reserve(aiMesh->mNumFaces * 3);
    for (unsigned f = 0; f < aiMesh->mNumFaces; f++) {
        const aiFace& face = aiMesh->mFaces[f];
        for (unsigned idx = 0; idx < face.mNumIndices; idx++) {
            srcIndices.push_back(face.mIndices[idx]);
        }
    }

    // Compute bounding sphere
    myMesh->localCenter = 0.5f * (myMesh->minBounds + myMesh->maxBounds);
    myMesh->boundingSphereRadius = glm::length(myMesh->maxBounds - myMesh->localCenter);

    int materialIndex = (int)aiMesh->mMaterialIndex;
    MeshTextures mt = LoadMeshTextures(scene->mMaterials[materialIndex], std::filesystem::path(m_FilePath).parent_path().string());

    // LOD generation:
    // Flatten positions to a float array
    std::vector<float> srcVertices;
    std::visit([&](const auto& positionsVec) {
        srcVertices.reserve(positionsVec.size() * 3);
        for (const auto& position : positionsVec) {
            if constexpr (std::is_same_v<std::decay_t<decltype(position)>, glm::vec3>) {
                srcVertices.push_back(position.x);
                srcVertices.push_back(position.y);
                srcVertices.push_back(position.z);
            }
            else {
                // If it's vec2, fill with z=0
                srcVertices.push_back(position.x);
                srcVertices.push_back(position.y);
                srcVertices.push_back(0.0f);
            }
        }
        }, myMesh->positions);

    std::vector<std::vector<uint32_t>> outLods;
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

    m_MeshInfos.push_back(MeshInfo{ mt, myMesh, materialIndex });
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
            vertices.data(), (size_t)vertexCount, sizeof(float) * 3,
            (size_t)targetIndicesCount, 0.02f);

        // If not much reduction, try sloppy simplification
        if ((size_t)(numOptIndices * 1.1f) > indices.size()) {
            if (lodLevel > 1) {
                numOptIndices = meshopt_simplifySloppy(
                    indices.data(),
                    indices.data(),
                    (uint32_t)indices.size(),
                    vertices.data(),
                    vertexCount,
                    sizeof(float) * 3,
                    (size_t)targetIndicesCount,
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
        meshopt_optimizeVertexCache(indices.data(), indices.data(), (uint32_t)indices.size(), vertexCount);

        Logger::GetLogger()->info("LOD{}: {} indices {}", lodLevel, numOptIndices, sloppy ? "[sloppy]" : "");
        lodLevel++;

        outLods.push_back(indices);
    }
}

MeshTextures Model::LoadMeshTextures(const aiMaterial* material, const std::string& directory)
{
    MeshTextures result;

    std::unordered_map<aiTextureType, TextureType> aiToMyTextureType = {
        { aiTextureType_DIFFUSE, TextureType::Albedo },
        { aiTextureType_NORMALS, TextureType::Normal },
        { aiTextureType_LIGHTMAP, TextureType::AO },
        { aiTextureType_UNKNOWN, TextureType::MetalRoughness },
        { aiTextureType_EMISSIVE, TextureType::Emissive }
    };

    for (const auto& [aiType, myType] : aiToMyTextureType) {
        unsigned int textureCount = material->GetTextureCount(aiType);
        for (unsigned int i = 0; i < textureCount; ++i) {
            aiString str;
            if (material->GetTexture(aiType, i, &str) == AI_SUCCESS) {
                std::string filename = std::filesystem::path(str.C_Str()).filename().string();
                std::string fullPath = (std::filesystem::path(directory) / filename).string();

                TextureData texData;
                if (!texData.LoadFromFile(fullPath)) {
                    Logger::GetLogger()->error("Failed to load texture: {}", fullPath);
                    continue;
                }

                if (texData.GetWidth() <= 0 || texData.GetHeight() <= 0) {
                    Logger::GetLogger()->error("Invalid texture dimensions for: {}", fullPath);
                    continue;
                }

                // If you want to create ITexture objects later, do it here.
                // For now, we just store TextureData in result.
                // result.textures[myType] = someOpenGLTextureCreatedFrom(texData);

                // If you only store TextureData, do nothing further for now.
            }
        }
    }

    return result;
}