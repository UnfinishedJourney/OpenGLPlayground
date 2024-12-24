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
    bool centerModel)
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
        aiProcess_OptimizeMeshes;

    const aiScene* scene = importer.ReadFile(filePath, importFlags);
    if (!scene || !scene->HasMeshes())
    {
        Logger::GetLogger()->error("Assimp failed to load: {}", filePath);
        return false;
    }

    // 1) Load the scene’s materials
    loadSceneMaterials(scene, matLayout);

    // 2) For each mesh, read the geometry into your Mesh class
    //    Also handle fallback materials if the material index is out-of-range.
    std::string directory = std::filesystem::path(filePath).parent_path().string();
    for (unsigned int i = 0; i < scene->mNumMeshes; i++)
    {
        aiMesh* aimesh = scene->mMeshes[i];
        processAssimpMesh(scene, aimesh, meshLayout, i, directory);
    }

    // 3) If centerModel == true, shift geometry so the bounding box is centered at origin
    if (centerModel)
    {
        centerAllMeshes();
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

    // Add the newly created material to the manager
    MaterialManager::GetInstance().AddMaterial(matName, mat);
    return matName;
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

    // 3) Determine which material is used. 
    //    If the mesh’s material index is out of range => fallback
    int matIndex = (int)aimesh->mMaterialIndex;
    std::string materialName;
    if (matIndex < 0 || matIndex >= (int)m_Data.createdMaterials.size())
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
        // e.g. set some random color or something:
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

    // If you want to load actual textures from the aiMaterial, do it here:
    //   loadMaterialTextures(scene->mMaterials[aimesh->mMaterialIndex], fallbackMat, ...);
    // Or handle it in createMaterialForAssimpMat(...). Up to you.

    // 4) Create final record
    BetterModelMeshData record;
    record.mesh = newMesh;
    record.materialName = materialName;
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
            (uint32_t)indices.size(),
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
                    (uint32_t)indices.size(),
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
        meshopt_optimizeVertexCache(indices.data(), indices.data(), (uint32_t)indices.size(), vertexCount);

        Logger::GetLogger()->info("LOD{}: {} indices {}", lodLevel, numOptIndices, sloppy ? "[sloppy]" : "");
        lodLevel++;

        outLods.push_back(indices);
    }
}

void BetterModelLoader::loadMaterialTextures(const aiMaterial* /*aiMat*/,
    std::shared_ptr<Material> /*material*/,
    const MaterialLayout& /*matLayout*/,
    const std::string& /*directory*/)
{
    // If you want to load actual textures from the disk and store them in the Material,
    // do it here, e.g.:
    //
    // - Check if matLayout.textures contains TextureType::Albedo => then search aiTextureType_DIFFUSE
    // - For each found texture path, load into a TextureData or create an ITexture, etc.
    // - Then call material->SetTexture(TextureType::Albedo, theTexturePointer);
    //
    // This is mostly the same approach as your older code in "LoadMeshTextures(...)",
    // but you’d tie it to the new Material.
}

std::string BetterModelLoader::createFallbackMaterialName()
{
    ++m_FallbackMaterialCounter;
    return "FallbackMaterial_" + std::to_string(m_FallbackMaterialCounter);
}

void BetterModelLoader::centerAllMeshes()
{
    // For demonstration, suppose we compute the overall bounding box of all sub-meshes,
    // then shift every vertex so the center is at (0,0,0).
    glm::vec3 sceneMin(FLT_MAX);
    glm::vec3 sceneMax(-FLT_MAX);

    // 1) gather the bounding box
    for (auto& rec : m_Data.meshesData)
    {
        auto& mesh = rec.mesh;
        sceneMin = glm::min(sceneMin, mesh->minBounds);
        sceneMax = glm::max(sceneMax, mesh->maxBounds);
    }
    glm::vec3 center = 0.5f * (sceneMin + sceneMax);

    // 2) shift each mesh’s vertex data
    for (auto& rec : m_Data.meshesData)
    {
        auto& mesh = rec.mesh;
        if (std::holds_alternative<std::vector<glm::vec3>>(mesh->positions))
        {
            auto& pvec = std::get<std::vector<glm::vec3>>(mesh->positions);
            for (auto& p : pvec)
            {
                p -= center;
            }
            // Recalc bounding box, localCenter
            mesh->minBounds -= center;
            mesh->maxBounds -= center;
            mesh->localCenter = 0.5f * (mesh->minBounds + mesh->maxBounds);
        }
    }
    Logger::GetLogger()->info("Centered all meshes around the origin.");
}