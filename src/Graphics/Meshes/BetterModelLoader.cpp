#include "BetterModelLoader.h"

// Assimp includes
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// meshoptimizer
#include <meshoptimizer.h>

// STL includes
#include <stdexcept>
#include <filesystem>
#include <cfloat>

// Engine includes
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

    // Setup Assimp Importer
    Assimp::Importer importer;
    unsigned int importFlags = aiProcess_JoinIdenticalVertices |
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_CalcTangentSpace |
        aiProcess_RemoveRedundantMaterials |
        aiProcess_FindInvalidData |
        aiProcess_OptimizeMeshes |
        aiProcess_ConvertToLeftHanded; // Adjust based on your engine's coordinate system

    const aiScene* scene = importer.ReadFile(filePath, importFlags);
    if (!scene || !scene->HasMeshes())
    {
        Logger::GetLogger()->error("Assimp failed to load model: {}", filePath);
        return false;
    }

    // Determine the directory path for textures
    std::string directory = std::filesystem::path(filePath).parent_path().string();

    // 1) Load the scene’s materials
    loadSceneMaterials(scene, matLayout, directory);

    // 2) Traverse and process nodes to build the SceneGraph
    processAssimpNode(scene, scene->mRootNode, meshLayout, matLayout, directory, sceneGraph, -1);

    // 3) If centerModel == true, shift geometry so the bounding box is centered at origin
    if (centerModel)
    {
        centerScene(sceneGraph);
    }

    Logger::GetLogger()->info(
        "BetterModelLoader: Successfully loaded model '{}' with {} meshes and {} materials.",
        filePath,
        m_Data.meshesData.size(),
        m_Data.createdMaterials.size()
    );
    return true;
}

void BetterModelLoader::loadSceneMaterials(const aiScene* scene, const MaterialLayout& matLayout, const std::string& directory)
{
    // Iterate through each material in the scene and create engine materials
    for (unsigned int i = 0; i < scene->mNumMaterials; i++)
    {
        aiMaterial* aiMat = scene->mMaterials[i];
        std::string matName = createMaterialForAssimpMat(aiMat, matLayout, directory);
        // Store the material name
        m_Data.createdMaterials.push_back(matName);
    }
}

std::string BetterModelLoader::createMaterialForAssimpMat(const aiMaterial* aiMat, const MaterialLayout& matLayout, const std::string& directory)
{
    // Retrieve material name from Assimp
    aiString ainame;
    if (AI_SUCCESS != aiMat->Get(AI_MATKEY_NAME, ainame))
    {
        ainame = aiString("UnnamedMaterial");
    }
    std::string matName = ainame.C_Str();

    // Ensure the material name is unique
    matName = ensureUniqueMaterialName(matName);

    // Check if the material already exists in the MaterialManager
    auto existingMat = MaterialManager::GetInstance().GetMaterialByName(matName);
    if (existingMat)
    {
        // Material already exists, reuse it
        return existingMat->GetName();
    }

    // Create a new Material instance
    auto mat = std::make_shared<Material>();
    mat->SetName(matName);

    // Load material properties (ambient, diffuse, specular, shininess)
    loadMaterialProperties(aiMat, mat, matLayout);

    // Load and assign textures to the material
    loadMaterialTextures(aiMat, mat, matLayout, directory);

    // Add the newly created material to the MaterialManager
    MaterialManager::GetInstance().AddMaterial(matName, matLayout, mat);

    return matName;
}

std::string BetterModelLoader::ensureUniqueMaterialName(const std::string& baseName)
{
    static std::unordered_set<std::string> materialNames;
    std::string uniqueName = baseName;
    size_t counter = 1;
    while (materialNames.find(uniqueName) != materialNames.end()) {
        uniqueName = baseName + "_" + std::to_string(counter++);
    }
    materialNames.insert(uniqueName);
    return uniqueName;
}

void BetterModelLoader::loadMaterialProperties(const aiMaterial* aiMat, std::shared_ptr<Material> mat, const MaterialLayout& matLayout)
{
    // Load ambient color
    if (matLayout.params.count(MaterialParamType::Ambient))
    {
        aiColor3D color(0.1f, 0.1f, 0.1f);  // Default ambient color
        aiMat->Get(AI_MATKEY_COLOR_AMBIENT, color);
        mat->SetParam(MaterialParamType::Ambient, glm::vec3(color.r, color.g, color.b));
    }

    // Load diffuse color
    if (matLayout.params.count(MaterialParamType::Diffuse))
    {
        aiColor3D color(0.5f, 0.5f, 0.5f);  // Default diffuse color
        aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
        mat->SetParam(MaterialParamType::Diffuse, glm::vec3(color.r, color.g, color.b));
    }

    // Load specular color
    if (matLayout.params.count(MaterialParamType::Specular))
    {
        aiColor3D color(0.5f, 0.5f, 0.5f);  // Default specular color
        aiMat->Get(AI_MATKEY_COLOR_SPECULAR, color);
        mat->SetParam(MaterialParamType::Specular, glm::vec3(color.r, color.g, color.b));
    }

    // Load shininess
    if (matLayout.params.count(MaterialParamType::Shininess))
    {
        float shininessVal = 32.0f;  // Default shininess
        aiMat->Get(AI_MATKEY_SHININESS, shininessVal);
        mat->SetParam(MaterialParamType::Shininess, shininessVal);
    }
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

                // Generate a unique texture name (you can customize this as needed)
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

            // Create and add the fallback material
            auto fallbackMat = createFallbackMaterial(materialName, matLayout);
            MaterialManager::GetInstance().AddMaterial(materialName, matLayout, fallbackMat);
            m_Data.createdMaterials.push_back(materialName);
        }
        else
        {
            materialName = m_Data.createdMaterials[matIndex];
        }

        // Assign the material name to the mesh data
        meshData.materialName = materialName;

        // Add mesh and material reference to the SceneGraph node
        sceneGraph.AddMeshReference(currentSGNode, static_cast<int>(meshIndex), matIndex);
    }

    // Recursively process child nodes
    for (unsigned int i = 0; i < ainode->mNumChildren; i++)
    {
        processAssimpNode(scene, ainode->mChildren[i], meshLayout, matLayout, directory, sceneGraph, currentSGNode);
    }

    // Set bounding volumes for the current node
    setNodeBoundingVolumes(scene, ainode, currentSGNode, sceneGraph);
}

std::string BetterModelLoader::createFallbackMaterialName()
{
    return "FallbackMaterial_" + std::to_string(++m_FallbackMaterialCounter);
}

std::shared_ptr<Material> BetterModelLoader::createFallbackMaterial(const std::string& name, const MaterialLayout& matLayout)
{
    auto fallbackMat = std::make_shared<Material>();
    fallbackMat->SetName(name);
    // Set default material properties
    fallbackMat->SetParam(MaterialParamType::Ambient, glm::vec3(0.2f, 0.2f, 0.2f));
    fallbackMat->SetParam(MaterialParamType::Diffuse, glm::vec3(0.6f, 0.6f, 0.6f));
    fallbackMat->SetParam(MaterialParamType::Specular, glm::vec3(0.5f, 0.5f, 0.5f));
    fallbackMat->SetParam(MaterialParamType::Shininess, 32.0f);
    return fallbackMat;
}

void BetterModelLoader::setNodeBoundingVolumes(const aiScene* scene, const aiNode* ainode, int currentSGNode, SceneGraph& sceneGraph)
{
    glm::vec3 nodeMin(FLT_MAX);
    glm::vec3 nodeMax(-FLT_MAX);

    // Aggregate bounding boxes from all meshes in this node
    for (unsigned int i = 0; i < ainode->mNumMeshes; i++)
    {
        unsigned int meshIndex = ainode->mMeshes[i];
        auto& meshData = m_Data.meshesData[meshIndex].mesh;

        nodeMin = glm::min(nodeMin, meshData->minBounds);
        nodeMax = glm::max(nodeMax, meshData->maxBounds);
    }

    if (ainode->mNumMeshes > 0)
    {
        glm::vec3 nodeCenter = 0.5f * (nodeMin + nodeMax);
        float nodeRadius = glm::length(nodeMax - nodeCenter);
        sceneGraph.SetNodeBoundingVolumes(currentSGNode, nodeMin, nodeMax, nodeCenter, nodeRadius);
    }
}

void BetterModelLoader::processAssimpMesh(
    const aiScene* scene,
    const aiMesh* aimesh,
    const MeshLayout& meshLayout,
    int meshIndex,
    const std::string& directory)
{
    // Create a new Mesh instance
    auto newMesh = std::make_shared<Mesh>();

    // 1) Load positions
    if (meshLayout.hasPositions && aimesh->HasPositions())
    {
        newMesh->positions.reserve(aimesh->mNumVertices);
        for (unsigned v = 0; v < aimesh->mNumVertices; v++)
        {
            glm::vec3 pos(aimesh->mVertices[v].x,
                aimesh->mVertices[v].y,
                aimesh->mVertices[v].z);
            newMesh->positions.push_back(pos);
            // Update bounding box
            newMesh->minBounds = glm::min(newMesh->minBounds, pos);
            newMesh->maxBounds = glm::max(newMesh->maxBounds, pos);
        }
    }

    // 2) Load normals
    if (meshLayout.hasNormals && aimesh->HasNormals())
    {
        newMesh->normals.reserve(aimesh->mNumVertices);
        for (unsigned v = 0; v < aimesh->mNumVertices; v++)
        {
            glm::vec3 normal(aimesh->mNormals[v].x,
                aimesh->mNormals[v].y,
                aimesh->mNormals[v].z);
            newMesh->normals.push_back(normal);
        }
    }

    // 3) Load tangents
    if (meshLayout.hasTangents && aimesh->HasTangentsAndBitangents())
    {
        newMesh->tangents.reserve(aimesh->mNumVertices);
        for (unsigned v = 0; v < aimesh->mNumVertices; v++)
        {
            glm::vec3 tangent(aimesh->mTangents[v].x,
                aimesh->mTangents[v].y,
                aimesh->mTangents[v].z);
            newMesh->tangents.push_back(tangent);
        }
    }

    // 4) Load bitangents
    if (meshLayout.hasBitangents && aimesh->HasTangentsAndBitangents())
    {
        newMesh->bitangents.reserve(aimesh->mNumVertices);
        for (unsigned v = 0; v < aimesh->mNumVertices; v++)
        {
            glm::vec3 bitangent(aimesh->mBitangents[v].x,
                aimesh->mBitangents[v].y,
                aimesh->mBitangents[v].z);
            newMesh->bitangents.push_back(bitangent);
        }
    }

    // 5) Load UVs
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
        // Assign the same UV set to all requested texture types
        for (auto textureType : meshLayout.textureTypes)
        {
            newMesh->uvs[textureType] = uvSet;
        }
    }

    // 6) Load indices
    std::vector<uint32_t> srcIndices;
    srcIndices.reserve(aimesh->mNumFaces * 3);
    for (unsigned f = 0; f < aimesh->mNumFaces; f++)
    {
        const aiFace& face = aimesh->mFaces[f];
        // Assuming the mesh is triangulated
        for (unsigned idx = 0; idx < face.mNumIndices; idx++)
        {
            srcIndices.push_back(face.mIndices[idx]);
        }
    }

    // 7) Setup bounding sphere
    newMesh->localCenter = 0.5f * (newMesh->minBounds + newMesh->maxBounds);
    newMesh->boundingSphereRadius = glm::length(newMesh->maxBounds - newMesh->localCenter);

    // 8) LOD generation
    std::vector<float> floatPositions;
    floatPositions.reserve(newMesh->positions.size() * 3);
    for (const auto& pos : newMesh->positions)
    {
        floatPositions.push_back(pos.x);
        floatPositions.push_back(pos.y);
        floatPositions.push_back(pos.z);
    }

    std::vector<std::vector<uint32_t>> lodIndices;
    generateLODs(srcIndices, floatPositions, lodIndices);

    // 9) Populate Mesh::indices and Mesh::lods
    newMesh->indices.clear();
    newMesh->lods.clear();
    for (const auto& singleLod : lodIndices)
    {
        MeshLOD lod;
        lod.indexOffset = static_cast<uint32_t>(newMesh->indices.size());
        lod.indexCount = static_cast<uint32_t>(singleLod.size());
        newMesh->indices.insert(newMesh->indices.end(), singleLod.begin(), singleLod.end());
        newMesh->lods.push_back(lod);
    }

    // 10) Store the mesh
    BetterModelMeshData record;
    record.mesh = newMesh;
    // Material name is handled in processAssimpNode
    m_Data.meshesData.push_back(record);
}

void BetterModelLoader::generateLODs(const std::vector<uint32_t>& srcIndices,
    const std::vector<float>& vertices3f,
    std::vector<std::vector<uint32_t>>& outLods) const
{
    if (srcIndices.empty() || vertices3f.empty()) {
        // No LOD if no data
        outLods.emplace_back(srcIndices);
        return;
    }

    size_t vertexCount = vertices3f.size() / 3;
    size_t currentIndexCount = srcIndices.size();
    outLods.emplace_back(srcIndices); // LOD0

    uint8_t lodLevel = 1;
    while (currentIndexCount > 1024 && lodLevel < 8) {
        size_t targetCount = currentIndexCount / 2;
        std::vector<uint32_t> simplifiedIndices(srcIndices);

        size_t numOptIndices = meshopt_simplify(
            simplifiedIndices.data(),
            simplifiedIndices.data(),
            static_cast<uint32_t>(simplifiedIndices.size()),
            vertices3f.data(),
            vertexCount,
            sizeof(float) * 3,
            targetCount,
            0.02f // Optional simplification threshold
        );

        bool sloppy = false;
        if (static_cast<size_t>(numOptIndices) > simplifiedIndices.size() * 0.9f) { // 10% tolerance
            if (lodLevel > 1) {
                numOptIndices = meshopt_simplifySloppy(
                    simplifiedIndices.data(),
                    simplifiedIndices.data(),
                    static_cast<uint32_t>(simplifiedIndices.size()),
                    vertices3f.data(),
                    vertexCount,
                    sizeof(float) * 3,
                    targetCount,
                    FLT_MAX,
                    nullptr
                );
                sloppy = true;
                if (numOptIndices == simplifiedIndices.size()) {
                    Logger::GetLogger()->warn("LOD{}: No further simplification possible.", lodLevel);
                    break;
                }
            }
            else {
                Logger::GetLogger()->warn("LOD{}: Simplification did not reduce indices enough.", lodLevel);
                break;
            }
        }

        simplifiedIndices.resize(numOptIndices);
        meshopt_optimizeVertexCache(simplifiedIndices.data(), simplifiedIndices.data(), static_cast<uint32_t>(simplifiedIndices.size()), vertexCount);

        Logger::GetLogger()->info("LOD{}: {} indices {}", lodLevel, numOptIndices, sloppy ? "[sloppy]" : "");
        outLods.emplace_back(simplifiedIndices);
        currentIndexCount = numOptIndices;
        lodLevel++;
    }
}

void BetterModelLoader::centerScene(SceneGraph& sceneGraph)
{
    if (sceneGraph.GetNodes().empty()) return;

    glm::vec3 sceneMin(FLT_MAX);
    glm::vec3 sceneMax(-FLT_MAX);

    // Gather overall bounding box from all meshes
    for (const auto& meshData : m_Data.meshesData)
    {
        sceneMin = glm::min(sceneMin, meshData.mesh->minBounds);
        sceneMax = glm::max(sceneMax, meshData.mesh->maxBounds);
    }

    glm::vec3 center = 0.5f * (sceneMin + sceneMax);
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), -center);

    // Apply the translation to the root node's local transform
    if (!sceneGraph.GetNodes().empty()) {
        sceneGraph.SetLocalTransform(0, translation * sceneGraph.GetNodes()[0].localTransform);
    }

    // Recalculate global transforms after modification
    sceneGraph.RecalculateGlobalTransforms();
}

glm::mat4 BetterModelLoader::AiToGlm(const aiMatrix4x4& m) {
    glm::mat4 r;
    r[0][0] = m.a1; r[1][0] = m.b1; r[2][0] = m.c1; r[3][0] = m.d1;
    r[0][1] = m.a2; r[1][1] = m.b2; r[2][1] = m.c2; r[3][1] = m.d2;
    r[0][2] = m.a3; r[1][2] = m.b3; r[2][2] = m.c3; r[3][2] = m.d3;
    r[0][3] = m.a4; r[1][3] = m.b4; r[2][3] = m.c4; r[3][3] = m.d4;
    return glm::transpose(r);
}

std::string BetterModelLoader::GetModelPath(const std::string& modelName) {
    static const std::unordered_map<std::string, std::string> modelPaths = {
        {"pig",    "../assets/Objs/pig_triangulated.obj"},
        {"bunny",  "../assets/Objs/bunny.obj"},
        {"dragon", "../assets/Objs/dragon.obj"},
        {"bistro", "../assets/AmazonBistro/Exterior/exterior.obj"},
        {"helmet", "../assets/DamagedHelmet/glTF/DamagedHelmet.gltf"}
    };

    auto it = modelPaths.find(modelName);
    if (it != modelPaths.end()) {
        return it->second;
    }
    return "";
}