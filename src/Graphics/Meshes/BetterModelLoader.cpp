#include "BetterModelLoader.h"

// Assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// meshoptimizer
#include <meshoptimizer.h>

// STL
#include <stdexcept>
#include <filesystem>
#include <cfloat>

// Your engine code
#include "Utilities/Logger.h"
#include "Resources/MaterialManager.h"
#include "Graphics/Materials/Material.h"

// Constructor and Destructor
BetterModelLoader::BetterModelLoader()
    : m_FallbackMaterialCounter(0)
{
}

BetterModelLoader::~BetterModelLoader()
{
}

bool BetterModelLoader::LoadModel(
    const std::string& filePath,
    const MeshLayout& meshLayout,
    const MaterialLayout& matLayout,
    bool centerModel,
    SceneGraph& sceneGraph)
{
    // Clear old data
    m_Data.meshesData.clear();
    m_Data.createdMaterials.clear();
    m_FallbackMaterialCounter = 0;

    // Setup Assimp
    Assimp::Importer importer;
    unsigned int importFlags = aiProcess_JoinIdenticalVertices |
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_CalcTangentSpace |
        aiProcess_RemoveRedundantMaterials |
        aiProcess_FindInvalidData |
        aiProcess_OptimizeMeshes |
        aiProcess_ConvertToLeftHanded; // Optional: depending on your engine's coordinate system

    const aiScene* scene = importer.ReadFile(filePath, importFlags);
    if (!scene || !scene->HasMeshes())
    {
        Logger::GetLogger()->error("Assimp failed to load: {}", filePath);
        return false;
    }

    // 1) Load the scene’s materials
    loadSceneMaterials(scene, matLayout);

    // 2) Traverse and process nodes to build the SceneGraph
    std::string directory = std::filesystem::path(filePath).parent_path().string();
    processAssimpNode(scene, scene->mRootNode, meshLayout, matLayout, directory, sceneGraph, -1);

    // 3) If centerModel == true, shift geometry so the bounding box is centered at origin
    if (centerModel)
    {
        // After building the scene graph, adjust transforms to center the model
        // This can be done by calculating the overall bounding box from all meshes
        // and adjusting the root node's transform
        // For simplicity, assume the root node is at index 0
        if (!sceneGraph.GetNodes().empty()) {
            glm::vec3 sceneMin(FLT_MAX);
            glm::vec3 sceneMax(-FLT_MAX);

            // Gather overall bounding box from all meshes
            for (const auto& meshData : m_Data.meshesData)
            {
                sceneMin = glm::min(sceneMin, meshData.mesh->minBounds);
                sceneMax = glm::max(sceneMax, meshData.mesh->maxBounds);
            }

            glm::vec3 center = 0.5f * (sceneMin + sceneMax);

            // Create a translation matrix to shift the scene
            glm::mat4 translation = glm::translate(glm::mat4(1.0f), -center);

            // Apply the translation to the root node's local transform
            // Assuming root node is at index 0
            if (!sceneGraph.GetNodes().empty()) {
                sceneGraph.SetLocalTransform(0, translation * sceneGraph.GetNodes()[0].localTransform);
            }

            // Recalculate global transforms after modification
            sceneGraph.RecalculateGlobalTransforms();

            // Optionally, update bounding volumes if needed
        }
    }

    Logger::GetLogger()->info(
        "BetterModelLoader: Loaded model '{}' with {} meshes and {} materials created.",
        filePath,
        scene->mNumMeshes,
        m_Data.createdMaterials.size()
    );
    return true;
}

void BetterModelLoader::loadSceneMaterials(const aiScene* scene, const MaterialLayout& matLayout)
{
    // For each aiMaterial, create a Material in the MaterialManager
    // (some might remain unused if a mesh doesn't reference them).
    for (unsigned int i = 0; i < scene->mNumMaterials; i++)
    {
        aiMaterial* aiMat = scene->mMaterials[i];
        std::string matName = createMaterialForAssimpMat(aiMat, matLayout);
        // Store that material name so we know it was created
        m_Data.createdMaterials.push_back(matName);
    }
}

std::string BetterModelLoader::createMaterialForAssimpMat(const aiMaterial* aiMat, const MaterialLayout& matLayout)
{
    // Retrieve name
    aiString ainame;
    if (AI_SUCCESS != aiMat->Get(AI_MATKEY_NAME, ainame))
    {
        ainame = aiString("UnnamedMaterial");
    }
    std::string matName = ainame.C_Str();

    // Check if MaterialManager already has this
    auto existingMat = MaterialManager::GetInstance().GetMaterial(matName);
    if (existingMat)
    {
        // Already exists, just reuse
        return existingMat->GetName();
    }

    // Otherwise, create a new Material
    auto mat = std::make_shared<Material>();
    mat->SetName(matName);

    // For each param in matLayout, try to load from aiMat
    if (matLayout.params.count(MaterialParamType::Ambient))
    {
        aiColor3D color(0.1f, 0.1f, 0.1f);  // default
        if (AI_SUCCESS == aiMat->Get(AI_MATKEY_COLOR_AMBIENT, color))
        {
            mat->SetParam(MaterialParamType::Ambient, glm::vec3(color.r, color.g, color.b));
        }
        else
        {
            mat->SetParam(MaterialParamType::Ambient, glm::vec3(0.1f));
        }
    }

    if (matLayout.params.count(MaterialParamType::Diffuse))
    {
        aiColor3D color(0.5f, 0.5f, 0.5f);
        if (AI_SUCCESS == aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color))
        {
            mat->SetParam(MaterialParamType::Diffuse, glm::vec3(color.r, color.g, color.b));
        }
        else
        {
            mat->SetParam(MaterialParamType::Diffuse, glm::vec3(0.5f));
        }
    }

    if (matLayout.params.count(MaterialParamType::Specular))
    {
        aiColor3D color(0.5f, 0.5f, 0.5f);
        if (AI_SUCCESS == aiMat->Get(AI_MATKEY_COLOR_SPECULAR, color))
        {
            mat->SetParam(MaterialParamType::Specular, glm::vec3(color.r, color.g, color.b));
        }
        else
        {
            mat->SetParam(MaterialParamType::Specular, glm::vec3(0.5f));
        }
    }

    if (matLayout.params.count(MaterialParamType::Shininess))
    {
        float shininessVal = 32.0f;
        if (AI_SUCCESS != aiMat->Get(AI_MATKEY_SHININESS, shininessVal))
        {
            // default
        }
        mat->SetParam(MaterialParamType::Shininess, shininessVal);
    }

    // Load textures if needed
    loadMaterialTextures(aiMat, mat, matLayout, ""); // Directory will be set later

    // Add the newly created material to the manager
    MaterialManager::GetInstance().AddMaterial(matName, mat);
    return matName;
}

void BetterModelLoader::processAssimpNode(
    const aiScene* scene,
    const aiNode* ainode,
    const MeshLayout& meshLayout,
    const MaterialLayout& matLayout,
    const std::string& directory,
    SceneGraph& sceneGraph,
    int parentNode)
{
    // Add a new node to the SceneGraph
    int currentSGNode = sceneGraph.AddNode(parentNode, ainode->mName.C_Str());

    // Convert Assimp's transformation to glm::mat4
    glm::mat4 localTransform = AiToGlm(ainode->mTransformation);
    sceneGraph.SetLocalTransform(currentSGNode, localTransform);

    // Process all meshes assigned to this node
    for (unsigned int i = 0; i < ainode->mNumMeshes; i++)
    {
        unsigned int meshIndex = ainode->mMeshes[i];
        aiMesh* aimesh = scene->mMeshes[meshIndex];
        // Process the mesh and store it
        processAssimpMesh(scene, aimesh, meshLayout, meshIndex, directory);
        // Retrieve the processed mesh data
        auto& meshData = m_Data.meshesData.back(); // Assuming processAssimpMesh adds to m_Data.meshesData
        // Find the material name
        int matIndex = aimesh->mMaterialIndex;
        std::string materialName;
        if (matIndex < 0 || matIndex >= static_cast<int>(m_Data.createdMaterials.size()))
        {
            // Create a fallback material for this mesh
            materialName = createFallbackMaterialName();
            Logger::GetLogger()->warn(
                "Mesh #{} does not have a valid material index => creating fallback '{}'.",
                meshIndex, materialName
            );

            // Actually add the fallback material in MaterialManager:
            auto fallbackMat = std::make_shared<Material>();
            fallbackMat->SetName(materialName);
            // e.g., set some default color
            fallbackMat->SetParam(MaterialParamType::Ambient, glm::vec3(0.2f, 0.2f, 0.2f));
            fallbackMat->SetParam(MaterialParamType::Diffuse, glm::vec3(0.6f, 0.6f, 0.6f));
            fallbackMat->SetParam(MaterialParamType::Specular, glm::vec3(0.5f));
            fallbackMat->SetParam(MaterialParamType::Shininess, 32.f);

            MaterialManager::GetInstance().AddMaterial(materialName, fallbackMat);
            m_Data.createdMaterials.push_back(materialName);
        }
        else
        {
            materialName = m_Data.createdMaterials[matIndex];
        }

        // Add mesh and material reference to the SceneGraph node
        sceneGraph.AddMeshReference(currentSGNode, static_cast<int>(meshIndex), matIndex);
    }

    // Recursively process child nodes
    for (unsigned int i = 0; i < ainode->mNumChildren; i++)
    {
        processAssimpNode(scene, ainode->mChildren[i], meshLayout, matLayout, directory, sceneGraph, currentSGNode);
    }

    // After processing all meshes and children, set bounding volumes if needed
    // Compute bounding volumes based on meshes assigned to this node
    glm::vec3 nodeMin(FLT_MAX);
    glm::vec3 nodeMax(-FLT_MAX);
    glm::vec3 nodeCenter(0.0f);
    float nodeRadius = 0.0f;

    for (unsigned int i = 0; i < ainode->mNumMeshes; i++)
    {
        unsigned int meshIndex = ainode->mMeshes[i];
        aiMesh* aimesh = scene->mMeshes[meshIndex];
        auto& meshData = m_Data.meshesData[meshIndex].mesh;

        nodeMin = glm::min(nodeMin, meshData->minBounds);
        nodeMax = glm::max(nodeMax, meshData->maxBounds);
    }

    if (ainode->mNumMeshes > 0)
    {
        nodeCenter = 0.5f * (nodeMin + nodeMax);
        nodeRadius = glm::length(nodeMax - nodeCenter);
        sceneGraph.SetNodeBoundingVolumes(currentSGNode, nodeMin, nodeMax, nodeCenter, nodeRadius);
    }
}

std::string BetterModelLoader::createFallbackMaterialName()
{
    ++m_FallbackMaterialCounter;
    return "FallbackMaterial_" + std::to_string(m_FallbackMaterialCounter);
}

void BetterModelLoader::processAssimpMesh(
    const aiScene* scene,
    const aiMesh* aimesh,
    const MeshLayout& meshLayout,
    int meshIndex,
    const std::string& directory)
{
    // 1) Create the Mesh
    auto newMesh = std::make_shared<Mesh>();

    // Positions
    if (meshLayout.hasPositions && aimesh->HasPositions())
    {
        std::vector<glm::vec3> positions;
        positions.reserve(aimesh->mNumVertices);
        for (unsigned v = 0; v < aimesh->mNumVertices; v++)
        {
            glm::vec3 pos(aimesh->mVertices[v].x,
                aimesh->mVertices[v].y,
                aimesh->mVertices[v].z);
            positions.push_back(pos);
            // Update bounding box
            newMesh->minBounds = glm::min(newMesh->minBounds, pos);
            newMesh->maxBounds = glm::max(newMesh->maxBounds, pos);
        }
        newMesh->positions = positions;
    }

    // Normals
    if (meshLayout.hasNormals && aimesh->HasNormals())
    {
        newMesh->normals.reserve(aimesh->mNumVertices);
        for (unsigned v = 0; v < aimesh->mNumVertices; v++)
        {
            newMesh->normals.emplace_back(
                aimesh->mNormals[v].x,
                aimesh->mNormals[v].y,
                aimesh->mNormals[v].z
            );
        }
    }

    // Tangents, Bitangents
    if (meshLayout.hasTangents && aimesh->HasTangentsAndBitangents())
    {
        newMesh->tangents.reserve(aimesh->mNumVertices);
        for (unsigned v = 0; v < aimesh->mNumVertices; v++)
        {
            newMesh->tangents.emplace_back(
                aimesh->mTangents[v].x,
                aimesh->mTangents[v].y,
                aimesh->mTangents[v].z
            );
        }
    }

    if (meshLayout.hasBitangents && aimesh->HasTangentsAndBitangents())
    {
        newMesh->bitangents.reserve(aimesh->mNumVertices);
        for (unsigned v = 0; v < aimesh->mNumVertices; v++)
        {
            newMesh->bitangents.emplace_back(
                aimesh->mBitangents[v].x,
                aimesh->mBitangents[v].y,
                aimesh->mBitangents[v].z
            );
        }
    }

    // UVs (we only load the first set, if available, for each requested textureType)
    if (!meshLayout.textureTypes.empty() && aimesh->HasTextureCoords(0))
    {
        std::vector<glm::vec2> uvSet(aimesh->mNumVertices);
        for (unsigned v = 0; v < aimesh->mNumVertices; v++)
        {
            uvSet[v] = glm::vec2(
                aimesh->mTextureCoords[0][v].x,
                aimesh->mTextureCoords[0][v].y
            );
        }
        // Assign the same UV to all texture types that require it
        for (auto t : meshLayout.textureTypes)
        {
            newMesh->uvs[t] = uvSet;
        }
    }

    // Indices
    std::vector<uint32_t> srcIndices;
    srcIndices.reserve(aimesh->mNumFaces * 3);
    for (unsigned f = 0; f < aimesh->mNumFaces; f++)
    {
        const aiFace& face = aimesh->mFaces[f];
        // Usually triangulated => face.mNumIndices = 3
        for (unsigned idx = 0; idx < face.mNumIndices; idx++)
        {
            srcIndices.push_back(face.mIndices[idx]);
        }
    }

    // Setup bounding sphere
    newMesh->localCenter = 0.5f * (newMesh->minBounds + newMesh->maxBounds);
    newMesh->boundingSphereRadius = glm::length(newMesh->maxBounds - newMesh->localCenter);

    // 2) LOD generation
    // Flatten positions to a float array (x, y, z for each vertex)
    std::vector<float> floatPositions;
    if (std::holds_alternative<std::vector<glm::vec3>>(newMesh->positions))
    {
        const auto& pvec = std::get<std::vector<glm::vec3>>(newMesh->positions);
        floatPositions.reserve(pvec.size() * 3);
        for (auto& p : pvec)
        {
            floatPositions.push_back(p.x);
            floatPositions.push_back(p.y);
            floatPositions.push_back(p.z);
        }
    }
    else
    {
        Logger::GetLogger()->warn("Mesh has positions in unexpected format. Skipping LOD generation.");
    }

    // Create multiple LOD index sets
    std::vector<std::vector<uint32_t>> lodIndices;
    generateLODs(srcIndices, floatPositions, lodIndices);

    // Put them into Mesh::indices and Mesh::lods
    newMesh->indices.clear();
    newMesh->lods.clear();
    for (auto& singleLod : lodIndices)
    {
        MeshLOD lod;
        lod.indexOffset = static_cast<uint32_t>(newMesh->indices.size());
        lod.indexCount = static_cast<uint32_t>(singleLod.size());
        newMesh->indices.insert(newMesh->indices.end(), singleLod.begin(), singleLod.end());
        newMesh->lods.push_back(lod);
    }

    // 3) Store the mesh
    BetterModelMeshData record;
    record.mesh = newMesh;
    // materialName is handled in processAssimpNode
    m_Data.meshesData.push_back(record);
}

void BetterModelLoader::generateLODs(std::vector<uint32_t>& indices,
    const std::vector<float>& vertices3f,
    std::vector<std::vector<uint32_t>>& outLods) const
{
    if (indices.empty() || vertices3f.empty())
    {
        // no LOD if no data
        outLods.push_back(indices);
        return;
    }

    size_t vertexCount = vertices3f.size() / 3;
    size_t targetCount = indices.size();

    Logger::GetLogger()->info("LOD0: {} indices", indices.size());
    outLods.push_back(indices);

    uint8_t lodLevel = 1;
    while (targetCount > 1024 && lodLevel < 8)
    {
        targetCount = indices.size() / 2;
        bool sloppy = false;

        size_t numOptIndices = meshopt_simplify(
            indices.data(),         // destination
            indices.data(),         // source
            static_cast<uint32_t>(indices.size()),
            vertices3f.data(),      // vertex positions
            vertexCount,
            sizeof(float) * 3,
            targetCount,
            0.02f // optional threshold
        );

        // If not enough reduction, try "sloppy"
        if ((size_t)(numOptIndices * 1.1f) > indices.size())
        {
            if (lodLevel > 1)
            {
                numOptIndices = meshopt_simplifySloppy(
                    indices.data(),
                    indices.data(),
                    static_cast<uint32_t>(indices.size()),
                    vertices3f.data(),
                    vertexCount,
                    sizeof(float) * 3,
                    targetCount,
                    FLT_MAX,
                    nullptr
                );
                sloppy = true;
                if (numOptIndices == indices.size())
                {
                    Logger::GetLogger()->warn("LOD{}: No further simplification possible.", lodLevel);
                    break;
                }
            }
            else
            {
                Logger::GetLogger()->warn("LOD{}: Simplification did not reduce indices enough.", lodLevel);
                break;
            }
        }

        indices.resize(numOptIndices);
        // Also reorder for better vertex cache usage
        meshopt_optimizeVertexCache(indices.data(), indices.data(), static_cast<uint32_t>(indices.size()), vertexCount);

        Logger::GetLogger()->info("LOD{}: {} indices {}", lodLevel, numOptIndices, sloppy ? "[sloppy]" : "");
        lodLevel++;

        outLods.push_back(indices);
    }
}

BetterMeshTextures BetterModelLoader::LoadMeshTextures(const aiMaterial* material, const std::string& directory)
{
    BetterMeshTextures result;

    // Mapping Assimp texture types to your engine's TextureType
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

                // Use filename as texture name or construct a unique name if needed
                std::string textureName = filename; // Alternatively, use matName + "_" + filename for uniqueness

                // Load texture via TextureManager
                auto texture = TextureManager::GetInstance().LoadTexture(textureName, fullPath);
                if (!texture) {
                    Logger::GetLogger()->error("Failed to load texture '{}' for type '{}'.", fullPath, static_cast<int>(myType));
                    continue;
                }

                result.textures[myType] = texture;
                Logger::GetLogger()->info("Texture '{}' loaded for type '{}'.", fullPath, static_cast<int>(myType));
            }
        }
    }

    return result;
}

void BetterModelLoader::loadMaterialTextures(const aiMaterial* aiMat,
    std::shared_ptr<Material> material,
    const MaterialLayout& matLayout,
    const std::string& directory)
{
    // Load textures using LoadMeshTextures
    BetterMeshTextures meshTextures = LoadMeshTextures(aiMat, directory);

    // Assign textures to the material
    for (const auto& [texType, texture] : meshTextures.textures) {
        material->SetTexture(texType, texture);
    }
}

glm::mat4 BetterModelLoader::AiToGlm(const aiMatrix4x4& m) {
    glm::mat4 r;
    r[0][0] = m.a1; r[1][0] = m.b1; r[2][0] = m.c1; r[3][0] = m.d1;
    r[0][1] = m.a2; r[1][1] = m.b2; r[2][1] = m.c2; r[3][1] = m.d2;
    r[0][2] = m.a3; r[1][2] = m.b3; r[2][2] = m.c3; r[3][2] = m.d3;
    r[0][3] = m.a4; r[1][3] = m.b4; r[2][3] = m.c4; r[3][3] = m.d4;
    return glm::transpose(r);
}