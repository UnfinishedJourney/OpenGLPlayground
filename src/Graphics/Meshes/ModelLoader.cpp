#include "ModelLoader.h"
#include <meshoptimizer.h>
#include <filesystem>
#include <cfloat>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/gtc/matrix_transform.hpp>

ModelLoader::ModelLoader()
    : m_FallbackMaterialCounter(0)
{
}

ModelLoader::~ModelLoader()
{
}

bool ModelLoader::LoadModel(const std::string& filePath,
    const MeshLayout& meshLayout,
    const MaterialLayout& matLayout,
    SceneGraph& sceneGraph,
    bool centerModel)
{
    m_Data.meshesData.clear();
    m_Data.createdMaterials.clear();
    m_FallbackMaterialCounter = 0;

    Assimp::Importer importer;
    unsigned int importFlags = aiProcess_JoinIdenticalVertices |
        aiProcess_Triangulate |
        aiProcess_RemoveRedundantMaterials |
        aiProcess_FindInvalidData |
        aiProcess_OptimizeMeshes;
    if (meshLayout.hasNormals) {
        importFlags |= aiProcess_GenSmoothNormals;
    }
    if (meshLayout.hasTangents || meshLayout.hasBitangents) {
        importFlags |= aiProcess_CalcTangentSpace;
    }

    const aiScene* scene = importer.ReadFile(filePath, importFlags);
    if (!scene || !scene->HasMeshes())
    {
        Logger::GetLogger()->error("Assimp failed to load model from '{}'.", filePath);
        return false;
    }

    // Load materials
    std::string directory = std::filesystem::path(filePath).parent_path().string();
    LoadSceneMaterials(scene, matLayout, directory);

    // Recursively process the scene’s node hierarchy
    ProcessAssimpNode(scene, scene->mRootNode, meshLayout, matLayout, directory, sceneGraph, -1);


    if (centerModel) {
        CenterScene(sceneGraph);
    }

    Logger::GetLogger()->info(
        "ModelLoader: Successfully loaded '{}' with {} meshes and {} materials.",
        filePath, m_Data.meshesData.size(), m_Data.createdMaterials.size()
    );
    return true;
}

void ModelLoader::LoadSceneMaterials(const aiScene* scene,
    const MaterialLayout& matLayout,
    const std::string& directory)
{
    for (unsigned int i = 0; i < scene->mNumMaterials; i++)
    {
        aiMaterial* aiMat = scene->mMaterials[i];
        std::string matName = CreateMaterialForAssimpMat(aiMat, matLayout, directory);
        m_Data.createdMaterials.push_back(matName);
    }
}

std::string ModelLoader::CreateMaterialForAssimpMat(const aiMaterial* aiMat,
    const MaterialLayout& matLayout,
    const std::string& directory)
{
    aiString aiName;
    if (AI_SUCCESS != aiMat->Get(AI_MATKEY_NAME, aiName))
    {
        aiName = aiString("UnnamedMaterial"); //maybe should rename it somehow to model name + counter
    }

    std::string matName = EnsureUniqueMaterialName(aiName.C_Str());
    auto existingMat = MaterialManager::GetInstance().GetMaterialByName(matName);
    if (existingMat) {
        // Material already exists, reuse
        return existingMat->GetName();
    }

    auto mat = std::make_shared<Material>();
    mat->SetName(matName);
    LoadMaterialProperties(aiMat, mat, matLayout);
    LoadMaterialTextures(aiMat, mat, matLayout, directory);

    MaterialManager::GetInstance().AddMaterial(matName, matLayout, mat);
    return matName;
}

std::string ModelLoader::EnsureUniqueMaterialName(const std::string& baseName)
{
    static std::unordered_set<std::string> usedNames;
    std::string uniqueName = baseName;
    size_t counter = 1;
    while (usedNames.find(uniqueName) != usedNames.end()) {
        uniqueName = baseName + "_" + std::to_string(counter++);
    }
    usedNames.insert(uniqueName);
    return uniqueName;
}

void ModelLoader::LoadMaterialProperties(const aiMaterial* aiMat,
    std::shared_ptr<Material> mat,
    const MaterialLayout& matLayout)
{
    // Example for Ambient
    if (matLayout.params.count(MaterialParamType::Ambient)) {
        aiColor3D color(0.1f, 0.1f, 0.1f);
        aiMat->Get(AI_MATKEY_COLOR_AMBIENT, color);
        mat->SetParam(MaterialParamType::Ambient, glm::vec3(color.r, color.g, color.b));
    }
    // Similarly for Diffuse, Specular, Shininess, etc.
    if (matLayout.params.count(MaterialParamType::Diffuse)) {
        aiColor3D color(0.5f, 0.5f, 0.5f);
        aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
        mat->SetParam(MaterialParamType::Diffuse, glm::vec3(color.r, color.g, color.b));
    }
    if (matLayout.params.count(MaterialParamType::Specular)) {
        aiColor3D color(0.5f, 0.5f, 0.5f);
        aiMat->Get(AI_MATKEY_COLOR_SPECULAR, color);
        mat->SetParam(MaterialParamType::Specular, glm::vec3(color.r, color.g, color.b));
    }
    if (matLayout.params.count(MaterialParamType::Shininess)) {
        float shininessVal = 32.0f;
        aiMat->Get(AI_MATKEY_SHININESS, shininessVal);
        mat->SetParam(MaterialParamType::Shininess, shininessVal);
    }
}

void ModelLoader::LoadMaterialTextures(const aiMaterial* aiMat,
    std::shared_ptr<Material> material,
    const MaterialLayout& matLayout,
    const std::string& directory)
{
    auto meshTextures = LoadMeshTextures(aiMat, directory);
    for (auto& [texType, texPtr] : meshTextures.textures) {
        material->SetTexture(texType, texPtr);
    }
}

BetterMeshTextures ModelLoader::LoadMeshTextures(const aiMaterial* material,
    const std::string& directory)
{
    BetterMeshTextures result;
    std::unordered_map<aiTextureType, TextureType> aiToMyType = {
        { aiTextureType_DIFFUSE,  TextureType::Albedo      },
        { aiTextureType_NORMALS,  TextureType::Normal      },
        { aiTextureType_LIGHTMAP, TextureType::AO          },
        { aiTextureType_UNKNOWN,  TextureType::MetalRoughness },
        { aiTextureType_EMISSIVE, TextureType::Emissive    }
    };

    for (auto& [aiType, myType] : aiToMyType) {
        unsigned int texCount = material->GetTextureCount(aiType);
        for (unsigned int i = 0; i < texCount; i++) {
            aiString str;
            if (material->GetTexture(aiType, i, &str) == AI_SUCCESS) {
                // OLD CODE: 
                // std::string filename = std::filesystem::path(str.C_Str()).filename().string();
                // std::string fullPath = (std::filesystem::path(directory) / filename).string();

                // NEW CODE:
                std::filesystem::path relativePath(str.C_Str());
                relativePath = relativePath.lexically_normal();
                std::filesystem::path fullPath = std::filesystem::path(directory) / relativePath;
                fullPath = fullPath.lexically_normal();

                auto texName = relativePath.filename().string(); // short name
                auto loadedTex = TextureManager::GetInstance().LoadTexture(texName, fullPath.string());
                if (!loadedTex) {
                    Logger::GetLogger()->error("Failed to load texture '{}' for type '{}'.",
                        fullPath.string(), (int)myType);
                    continue;
                }
                result.textures[myType] = loadedTex;
                Logger::GetLogger()->info("Texture '{}' loaded (type {}).", fullPath.string(), (int)myType);
            }
        }
    }
    return result;
}

std::string ModelLoader::CreateFallbackMaterialName()
{
    return "FallbackMat_" + std::to_string(++m_FallbackMaterialCounter);
}

//fallback material should be assosiated with layout
std::shared_ptr<Material> ModelLoader::CreateFallbackMaterial(const std::string& name,
    const MaterialLayout& matLayout)
{
    auto fallbackMat = std::make_shared<Material>();
    fallbackMat->SetName(name);
    fallbackMat->SetParam(MaterialParamType::Ambient, glm::vec3(0.2f));
    fallbackMat->SetParam(MaterialParamType::Diffuse, glm::vec3(0.6f));
    fallbackMat->SetParam(MaterialParamType::Specular, glm::vec3(0.5f));
    fallbackMat->SetParam(MaterialParamType::Shininess, 32.0f);
    return fallbackMat;
}

void ModelLoader::SetNodeBoundingVolumes(const aiScene* scene,
    const aiNode* ainode,
    int currentSGNode,
    SceneGraph& sceneGraph)
{
    glm::vec3 nodeMin(FLT_MAX);
    glm::vec3 nodeMax(-FLT_MAX);

    // Aggregate bounding boxes from all meshes in this node
    for (unsigned int i = 0; i < ainode->mNumMeshes; i++) {
        unsigned int meshIdx = ainode->mMeshes[i];
        if (meshIdx >= m_Data.meshesData.size()) continue;
        auto& meshPtr = m_Data.meshesData[meshIdx].mesh;

        nodeMin = glm::min(nodeMin, meshPtr->minBounds);
        nodeMax = glm::max(nodeMax, meshPtr->maxBounds);
    }
    if (ainode->mNumMeshes > 0) {
        glm::vec3 center = 0.5f * (nodeMin + nodeMax);
        float radius = glm::length(nodeMax - center);
        sceneGraph.SetNodeBoundingVolumes(currentSGNode, nodeMin, nodeMax, center, radius);
    }
}

void ModelLoader::ProcessAssimpNode(const aiScene* scene,
    const aiNode* ainode,
    const MeshLayout& meshLayout,
    const MaterialLayout& matLayout,
    const std::string& directory,
    SceneGraph& sceneGraph,
    int parentNode)
{
    // Create a new node
    int currentSGNode = sceneGraph.AddNode(parentNode, ainode->mName.C_Str());

    // Convert transform to glm
    glm::mat4 localTransform = AiToGlm(ainode->mTransformation);
    sceneGraph.SetLocalTransform(currentSGNode, localTransform);

    // Process each mesh
    for (unsigned int i = 0; i < ainode->mNumMeshes; i++) {
        unsigned int meshIndex = ainode->mMeshes[i];
        aiMesh* aimesh = scene->mMeshes[meshIndex];

        // Convert the aiMesh => our engine’s Mesh
        ProcessAssimpMesh(scene, aimesh, meshLayout, meshIndex, directory);

        // Retrieve the processed mesh data
        auto& meshData = m_Data.meshesData.back();

        // figure out the material
        int matIndex = aimesh->mMaterialIndex;
        std::string materialName;
        if (matIndex < 0 || matIndex >= (int)m_Data.createdMaterials.size()) {
            // fallback
            materialName = CreateFallbackMaterialName();
            auto fallback = CreateFallbackMaterial(materialName, matLayout);
            MaterialManager::GetInstance().AddMaterial(materialName, matLayout, fallback);
            m_Data.createdMaterials.push_back(materialName);
        }
        else {
            materialName = m_Data.createdMaterials[matIndex];
        }
        meshData.materialName = materialName;

        // Add references to SceneGraph
        sceneGraph.AddMeshReference(currentSGNode, (int)meshIndex, matIndex);
    }

    // Recursively process children
    for (unsigned int c = 0; c < ainode->mNumChildren; c++) {
        ProcessAssimpNode(scene, ainode->mChildren[c],
            meshLayout, matLayout, directory,
            sceneGraph, currentSGNode);
    }

    // Compute bounding volumes
    SetNodeBoundingVolumes(scene, ainode, currentSGNode, sceneGraph);
}

void ModelLoader::ProcessAssimpMesh(const aiScene* scene,
    const aiMesh* aimesh,
    const MeshLayout& meshLayout,
    int meshIndex,
    const std::string& directory)
{
    // Create a new Mesh
    auto newMesh = std::make_shared<Mesh>();

    // Positions
    if (meshLayout.hasPositions && aimesh->HasPositions()) {
        newMesh->positions.reserve(aimesh->mNumVertices);
        for (unsigned v = 0; v < aimesh->mNumVertices; v++) {
            glm::vec3 pos(aimesh->mVertices[v].x,
                aimesh->mVertices[v].y,
                aimesh->mVertices[v].z);
            newMesh->positions.push_back(pos);
            newMesh->minBounds = glm::min(newMesh->minBounds, pos);
            newMesh->maxBounds = glm::max(newMesh->maxBounds, pos);
        }
    }

    // Normals
    if (meshLayout.hasNormals && aimesh->HasNormals()) {
        newMesh->normals.reserve(aimesh->mNumVertices);
        for (unsigned v = 0; v < aimesh->mNumVertices; v++) {
            glm::vec3 normal(aimesh->mNormals[v].x,
                aimesh->mNormals[v].y,
                aimesh->mNormals[v].z);
            newMesh->normals.push_back(normal);
        }
    }

    // Tangents/Bitangents
    if (meshLayout.hasTangents && aimesh->HasTangentsAndBitangents()) {
        newMesh->tangents.reserve(aimesh->mNumVertices);
        for (unsigned v = 0; v < aimesh->mNumVertices; v++) {
            glm::vec3 t(aimesh->mTangents[v].x,
                aimesh->mTangents[v].y,
                aimesh->mTangents[v].z);
            newMesh->tangents.push_back(t);
        }
    }
    if (meshLayout.hasBitangents && aimesh->HasTangentsAndBitangents()) {
        newMesh->bitangents.reserve(aimesh->mNumVertices);
        for (unsigned v = 0; v < aimesh->mNumVertices; v++) {
            glm::vec3 b(aimesh->mBitangents[v].x,
                aimesh->mBitangents[v].y,
                aimesh->mBitangents[v].z);
            newMesh->bitangents.push_back(b);
        }
    }

    // UVs (currently we just replicate UV0 across all requested texture channels)
    if (!meshLayout.textureTypes.empty() && aimesh->HasTextureCoords(0)) {
        std::vector<glm::vec2> uvSet(aimesh->mNumVertices);
        for (unsigned v = 0; v < aimesh->mNumVertices; v++) {
            uvSet[v] = glm::vec2(aimesh->mTextureCoords[0][v].x,
                aimesh->mTextureCoords[0][v].y);
        }
        for (auto texType : meshLayout.textureTypes) {
            newMesh->uvs[texType] = uvSet;
        }
    }

    // Gather indices
    std::vector<uint32_t> srcIndices;
    srcIndices.reserve(aimesh->mNumFaces * 3);
    for (unsigned f = 0; f < aimesh->mNumFaces; f++) {
        const aiFace& face = aimesh->mFaces[f];
        for (unsigned idx = 0; idx < face.mNumIndices; idx++) {
            srcIndices.push_back(face.mIndices[idx]);
        }
    }

    newMesh->localCenter = 0.5f * (newMesh->minBounds + newMesh->maxBounds);
    newMesh->boundingSphereRadius = glm::length(newMesh->maxBounds - newMesh->localCenter);

    // Prepare positions array for meshoptimizer
    std::vector<float> floatPositions;
    floatPositions.reserve(newMesh->positions.size() * 3);
    for (auto& pos : newMesh->positions) {
        floatPositions.push_back(pos.x);
        floatPositions.push_back(pos.y);
        floatPositions.push_back(pos.z);
    }

    // Generate LODs
    std::vector<std::vector<uint32_t>> lodIndices;
    GenerateLODs(srcIndices, floatPositions, lodIndices);

    // Store them in the mesh
    newMesh->indices.clear();
    newMesh->lods.clear();
    for (auto& singleLOD : lodIndices) {
        MeshLOD lod;
        lod.indexOffset = static_cast<uint32_t>(newMesh->indices.size());
        lod.indexCount = static_cast<uint32_t>(singleLOD.size());
        newMesh->indices.insert(newMesh->indices.end(), singleLOD.begin(), singleLOD.end());
        newMesh->lods.push_back(lod);
    }

    // Finally, record it in m_Data
    BetterModelMeshData record;
    record.mesh = newMesh;
    record.materialName.clear(); // assigned later
    m_Data.meshesData.push_back(record);
}

void ModelLoader::GenerateLODs(const std::vector<uint32_t>& srcIndices,
    const std::vector<float>& vertices3f,
    std::vector<std::vector<uint32_t>>& outLods) const
{
    if (srcIndices.empty() || vertices3f.empty()) {
        outLods.emplace_back(srcIndices); // just store what we have
        return;
    }
    size_t vertexCount = vertices3f.size() / 3;
    size_t currentIndexCount = srcIndices.size();
    outLods.emplace_back(srcIndices); // LOD0

    uint8_t lodLevel = 1;
    while (currentIndexCount > 1024 && lodLevel < 8) {
        size_t targetCount = currentIndexCount / 2;
        std::vector<uint32_t> simplified(srcIndices);

        size_t numOptIndices = meshopt_simplify(
            simplified.data(),
            simplified.data(),
            static_cast<uint32_t>(simplified.size()),
            vertices3f.data(),
            vertexCount,
            sizeof(float) * 3,
            targetCount,
            0.02f
        );
        bool sloppy = false;
        if (numOptIndices > simplified.size() * 0.9f) {
            // try sloppy
            if (lodLevel > 1) {
                numOptIndices = meshopt_simplifySloppy(
                    simplified.data(),
                    simplified.data(),
                    static_cast<uint32_t>(simplified.size()),
                    vertices3f.data(),
                    vertexCount,
                    sizeof(float) * 3,
                    targetCount,
                    FLT_MAX,
                    nullptr
                );
                sloppy = true;
                if (numOptIndices == simplified.size()) {
                    Logger::GetLogger()->warn("LOD{}: No further simplification possible.", lodLevel);
                    break;
                }
            }
            else {
                Logger::GetLogger()->warn("LOD{}: Simplification didn't reduce enough.", lodLevel);
                break;
            }
        }
        simplified.resize(numOptIndices);
        meshopt_optimizeVertexCache(
            simplified.data(),
            simplified.data(),
            static_cast<uint32_t>(numOptIndices),
            vertexCount
        );

        currentIndexCount = numOptIndices;
        outLods.emplace_back(simplified);

        Logger::GetLogger()->info("LOD{} => {} indices {}",
            lodLevel, numOptIndices, sloppy ? "[sloppy]" : "");
        lodLevel++;
    }
}

void ModelLoader::CenterScene(SceneGraph& sceneGraph)
{
    // If you want to shift the entire scene so that bounding box is centered at origin
    if (sceneGraph.GetNodes().empty()) return;

    glm::vec3 sceneMin(FLT_MAX);
    glm::vec3 sceneMax(-FLT_MAX);

    for (auto& meshData : m_Data.meshesData)
    {
        sceneMin = glm::min(sceneMin, meshData.mesh->minBounds);
        sceneMax = glm::max(sceneMax, meshData.mesh->maxBounds);
    }
    glm::vec3 center = 0.5f * (sceneMin + sceneMax);
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), -center);

    // Apply to the root node's local transform
    auto& nodes = sceneGraph.GetNodes();
    if (!nodes.empty()) {
        sceneGraph.SetLocalTransform(0, translation * nodes[0].localTransform);
    }
    sceneGraph.RecalculateGlobalTransforms();
}

glm::mat4 ModelLoader::AiToGlm(const aiMatrix4x4& m)
{
    glm::mat4 r;
    r[0][0] = m.a1; r[1][0] = m.b1; r[2][0] = m.c1; r[3][0] = m.d1;
    r[0][1] = m.a2; r[1][1] = m.b2; r[2][1] = m.c2; r[3][1] = m.d2;
    r[0][2] = m.a3; r[1][2] = m.b3; r[2][2] = m.c3; r[3][2] = m.d3;
    r[0][3] = m.a4; r[1][3] = m.b4; r[2][3] = m.c4; r[3][3] = m.d4;
    return glm::transpose(r);
}

std::string ModelLoader::GetModelPath(const std::string& modelName)
{
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