#include "StaticModelLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <filesystem>
#include <meshoptimizer.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp> // if needed

#include "Graphics/Materials/MaterialManager.h"
#include "Graphics/Textures/TextureManager.h"
#include "Graphics/Materials/Material.h"
#include "Utilities/Logger.h"
#include "MeshInfo.h"

namespace staticloader {

    ModelLoader::ModelLoader(float scaleFactor,
        std::unordered_map<aiTextureType, TextureType> aiToMyType,
        uint8_t maxLODs)
        : m_ScaleFactor(scaleFactor)
        , m_AiToMyType(std::move(aiToMyType))
        , m_MaxLODs(maxLODs)
    {
    }

    bool ModelLoader::LoadStaticModel(const std::string& modelName,
        const MeshLayout& meshLayout,
        const MaterialLayout& matLayout,
        bool centerModel)
    {
        // Clear previous data
        m_Objects.clear();
        m_MaterialIDs.clear();
        m_FallbackMaterialCounter = 0;
        m_UnnamedMaterialCounter = 0;

        // Configure Assimp
        Assimp::Importer importer;
        unsigned importFlags = aiProcess_JoinIdenticalVertices |
            aiProcess_Triangulate |
            aiProcess_RemoveRedundantMaterials |
            aiProcess_FindInvalidData |
            aiProcess_OptimizeMeshes; // merges small meshes

        if (meshLayout.hasNormals) {
            importFlags |= aiProcess_GenSmoothNormals;
        }
        if (meshLayout.hasTangents || meshLayout.hasBitangents) {
            importFlags |= aiProcess_CalcTangentSpace;
        }

        // Look up path
        std::string filePath = GetModelPath(modelName);
        if (filePath.empty()) {
            Logger::GetLogger()->error("ModelLoader: No path for model '{}'.", modelName);
            return false;
        }

        // Load scene
        const aiScene* scene = importer.ReadFile(filePath, importFlags);
        if (!scene || !scene->HasMeshes()) {
            Logger::GetLogger()->error("ModelLoader: Failed to load '{}'.", filePath);
            return false;
        }

        // Directory for textures
        std::string directory = std::filesystem::path(filePath).parent_path().string();

        // 1) Load materials from the scene
        LoadSceneMaterials(scene, matLayout, directory);

        // 2) Traverse node hierarchy iteratively
        std::vector<std::pair<aiNode*, glm::mat4>> stack;
        stack.push_back({ scene->mRootNode, glm::mat4(1.0f) });

        while (!stack.empty()) {
            auto [node, parentXform] = stack.back();
            stack.pop_back();

            glm::mat4 local = AiToGlm(node->mTransformation);
            glm::mat4 global = parentXform * local;

            // For each mesh index in this node
            for (unsigned i = 0; i < node->mNumMeshes; i++) {
                unsigned meshIndex = node->mMeshes[i];
                aiMesh* aimesh = scene->mMeshes[meshIndex];

                // Bake transform
                auto newMesh = ProcessAssimpMesh(aimesh, meshLayout, global);

                // Material
                int matID = -1;
                int matIndex = aimesh->mMaterialIndex;
                if (matIndex < 0 || matIndex >= (int)m_MaterialIDs.size()) {
                    // fallback
                    auto fallbackMat = std::make_unique<Graphics::Material>(matLayout);
                    fallbackMat->SetName("FallbackMat_" + std::to_string(++m_FallbackMaterialCounter));
                    auto id = Graphics::MaterialManager::GetInstance().AddMaterial(std::move(fallbackMat));
                    m_MaterialIDs.push_back(id.value());
                    matID = id.value();
                }
                else {
                    matID = m_MaterialIDs[matIndex];
                }

                // Store
                MeshInfo mi;
                mi.mesh = newMesh;
                mi.materialIndex = matID;
                m_Objects.push_back(mi);
            }

            // Push children
            for (unsigned c = 0; c < node->mNumChildren; c++) {
                stack.push_back({ node->mChildren[c], global });
            }
        }

        // 3) Optionally recenter geometry
        if (centerModel) {
            CenterMeshes();
        }

        Logger::GetLogger()->info("ModelLoader: Loaded '{}' => {} meshes, {} materials.",
            modelName, m_Objects.size(), m_MaterialIDs.size());
        return true;
    }

    void ModelLoader::LoadSceneMaterials(const aiScene* scene,
        const MaterialLayout& matLayout,
        const std::string& directory)
    {
        m_MaterialIDs.reserve(scene->mNumMaterials);
        for (unsigned i = 0; i < scene->mNumMaterials; i++) {
            aiMaterial* aimat = scene->mMaterials[i];
            auto matID = CreateMaterialForAssimpMat(aimat, matLayout, directory);
            m_MaterialIDs.push_back(matID);
        }
    }

    int ModelLoader::CreateMaterialForAssimpMat(const aiMaterial* aiMat,
        const MaterialLayout& matLayout,
        const std::string& directory)
    {
        aiString aiName;
        if (AI_SUCCESS != aiMat->Get(AI_MATKEY_NAME, aiName)) {
            aiName = aiString(("UnnamedMat_" + std::to_string(m_UnnamedMaterialCounter++)).c_str());
        }
        std::string matName(aiName.C_Str());

        // Possibly reuse existing material
        auto matID = Graphics::MaterialManager::GetInstance().GetMaterialIDByName(matName);
        if (matID.has_value()) {
            return matID.value();
        }

        auto material = std::make_unique<Graphics::Material>(matLayout);
        material->SetName(matName);

        // Load color/floats
        LoadMaterialProperties(aiMat, material, matLayout);

        // Load texture references
        LoadMaterialTextures(aiMat, material, matLayout, directory);

        // Register
        return Graphics::MaterialManager::GetInstance().AddMaterial(std::move(material)).value();
    }

    void ModelLoader::LoadMaterialProperties(const aiMaterial* aiMat,
        const std::unique_ptr<Graphics::Material>& mat,
        const MaterialLayout& matLayout)
    {
        // E.g. ambient, diffuse, specular, shininess
        if (matLayout.params_.count(MaterialParamType::Ambient)) {
            aiColor3D color(0.2f);
            aiMat->Get(AI_MATKEY_COLOR_AMBIENT, color);
            mat->AssignToPackedParams(MaterialParamType::Ambient, glm::vec3(color.r, color.g, color.b));
        }
        if (matLayout.params_.count(MaterialParamType::Diffuse)) {
            aiColor3D color(0.8f);
            aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
            mat->AssignToPackedParams(MaterialParamType::Diffuse, glm::vec3(color.r, color.g, color.b));
        }
        if (matLayout.params_.count(MaterialParamType::Specular)) {
            aiColor3D color(0.0f);
            aiMat->Get(AI_MATKEY_COLOR_SPECULAR, color);
            mat->AssignToPackedParams(MaterialParamType::Specular, glm::vec3(color.r, color.g, color.b));
        }
        if (matLayout.params_.count(MaterialParamType::Shininess)) {
            float shininess = 32.0f;
            aiMat->Get(AI_MATKEY_SHININESS, shininess);
            mat->AssignToPackedParams(MaterialParamType::Shininess, shininess);
        }
    }

    void ModelLoader::LoadMaterialTextures(const aiMaterial* aiMat,
        const std::unique_ptr<Graphics::Material>& mat,
        const MaterialLayout& matLayout,
        const std::string& directory)
    {
        for (auto& [aiType, myType] : m_AiToMyType) {
            unsigned count = aiMat->GetTextureCount(aiType);
            for (unsigned i = 0; i < count; i++) {
                aiString texPath;
                if (aiMat->GetTexture(aiType, i, &texPath) == AI_SUCCESS) {
                    // Build path
                    std::filesystem::path rel(texPath.C_Str());
                    rel = rel.lexically_normal();
                    std::filesystem::path full = std::filesystem::path(directory) / rel;
                    full = full.lexically_normal();

                    // Load
                    auto textureName = rel.filename().string();
                    auto loadedTex = Graphics::TextureManager::GetInstance().LoadTexture(textureName, full.string());
                    if (!loadedTex) {
                        Logger::GetLogger()->error("Failed to load texture '{}' for type={}.",
                            full.string(), (int)myType);
                        continue;
                    }
                    mat->SetTexture(myType, loadedTex);
                    Logger::GetLogger()->info("Loaded texture '{}' (type={}).", full.string(), (int)myType);
                }
            }
        }
    }

    std::shared_ptr<Mesh> ModelLoader::ProcessAssimpMesh(const aiMesh* aimesh,
        const MeshLayout& meshLayout,
        const glm::mat4& transform)
    {
        auto mesh = std::make_shared<Mesh>();

        // Positions
        if (meshLayout.hasPositions && aimesh->HasPositions()) {
            mesh->positions.reserve(aimesh->mNumVertices);
            glm::vec4 tmp;
            for (unsigned v = 0; v < aimesh->mNumVertices; v++) {
                tmp.x = m_ScaleFactor * aimesh->mVertices[v].x;
                tmp.y = m_ScaleFactor * aimesh->mVertices[v].y;
                tmp.z = m_ScaleFactor * aimesh->mVertices[v].z;
                tmp.w = 1.0f;
                glm::vec4 worldPos = transform * tmp;
                glm::vec3 finalPos(worldPos.x, worldPos.y, worldPos.z);

                mesh->positions.push_back(finalPos);

                // Update bounding box
                mesh->minBounds = glm::min(mesh->minBounds, finalPos);
                mesh->maxBounds = glm::max(mesh->maxBounds, finalPos);
            }
        }

        // Normals
        if (meshLayout.hasNormals && aimesh->HasNormals()) {
            mesh->normals.reserve(aimesh->mNumVertices);
            glm::mat3 normalMat = glm::mat3(glm::transpose(glm::inverse(transform)));
            for (unsigned v = 0; v < aimesh->mNumVertices; v++) {
                glm::vec3 n(aimesh->mNormals[v].x,
                    aimesh->mNormals[v].y,
                    aimesh->mNormals[v].z);
                n = glm::normalize(normalMat * n);
                mesh->normals.push_back(n);
            }
        }

        // Tangents/Bitangents
        if ((meshLayout.hasTangents || meshLayout.hasBitangents) &&
            aimesh->HasTangentsAndBitangents())
        {
            glm::mat3 tbMat = glm::mat3(glm::transpose(glm::inverse(transform)));

            if (meshLayout.hasTangents) {
                mesh->tangents.reserve(aimesh->mNumVertices);
            }
            if (meshLayout.hasBitangents) {
                mesh->bitangents.reserve(aimesh->mNumVertices);
            }

            for (unsigned v = 0; v < aimesh->mNumVertices; v++) {
                if (meshLayout.hasTangents) {
                    glm::vec3 t(aimesh->mTangents[v].x,
                        aimesh->mTangents[v].y,
                        aimesh->mTangents[v].z);
                    t = glm::normalize(tbMat * t);
                    mesh->tangents.push_back(t);
                }
                if (meshLayout.hasBitangents) {
                    glm::vec3 b(aimesh->mBitangents[v].x,
                        aimesh->mBitangents[v].y,
                        aimesh->mBitangents[v].z);
                    b = glm::normalize(tbMat * b);
                    mesh->bitangents.push_back(b);
                }
            }
        }

        // UVs (just replicate channel 0 to all requested texture sets)
        if (!meshLayout.textureTypes.empty() && aimesh->HasTextureCoords(0)) {
            std::vector<glm::vec2> uvSet(aimesh->mNumVertices);
            for (unsigned v = 0; v < aimesh->mNumVertices; v++) {
                uvSet[v] = glm::vec2(aimesh->mTextureCoords[0][v].x,
                    aimesh->mTextureCoords[0][v].y);
            }
            for (auto txType : meshLayout.textureTypes) {
                mesh->uvs[txType] = uvSet;
            }
        }

        // Indices
        std::vector<uint32_t> srcIndices;
        srcIndices.reserve(aimesh->mNumFaces * 3);
        for (unsigned f = 0; f < aimesh->mNumFaces; f++) {
            const aiFace& face = aimesh->mFaces[f];
            // Triangulated => 3 indices
            for (unsigned idx = 0; idx < face.mNumIndices; idx++) {
                srcIndices.push_back(face.mIndices[idx]);
            }
        }

        // bounding volume
        mesh->localCenter = 0.5f * (mesh->minBounds + mesh->maxBounds);
        mesh->boundingSphereRadius = glm::length(mesh->maxBounds - mesh->localCenter);

        // Prepare for LOD generation
        std::vector<float> floatPositions;
        floatPositions.reserve(mesh->positions.size() * 3);
        for (auto& pos : mesh->positions) {
            floatPositions.push_back(pos.x);
            floatPositions.push_back(pos.y);
            floatPositions.push_back(pos.z);
        }

        // LOD creation
        std::vector<std::vector<uint32_t>> lodIndices;
        GenerateLODs(std::move(srcIndices), floatPositions, lodIndices);

        mesh->indices.clear();
        mesh->lods.clear();

        for (auto& singleLOD : lodIndices) {
            MeshLOD lod;
            lod.indexOffset = static_cast<uint32_t>(mesh->indices.size());
            lod.indexCount = static_cast<uint32_t>(singleLOD.size());
            mesh->indices.insert(mesh->indices.end(), singleLOD.begin(), singleLOD.end());
            mesh->lods.push_back(lod);
        }

        return mesh;
    }

    void ModelLoader::GenerateLODs(std::vector<uint32_t> srcIndices,
        const std::vector<float>& vertices3f,
        std::vector<std::vector<uint32_t>>& outLods) const
    {
        if (srcIndices.empty() || vertices3f.empty()) {
            outLods.push_back(std::move(srcIndices));
            return;
        }

        const size_t vertexCount = vertices3f.size() / 3;
        size_t       currentIndexCount = srcIndices.size();

        // LOD0 => original
        outLods.emplace_back(srcIndices);

        // Attempt LOD1.. up to m_MaxLODs
        size_t lodLevel = 1;
        while (lodLevel < m_MaxLODs && currentIndexCount > 1024) {
            size_t targetCount = currentIndexCount / 2;

            const auto& prevLOD = outLods.back();
            std::vector<uint32_t> simplified(prevLOD);

            // Simplify
            size_t numOpt = meshopt_simplify(
                simplified.data(),
                simplified.data(),
                static_cast<uint32_t>(simplified.size()),
                vertices3f.data(),
                static_cast<uint32_t>(vertexCount),
                sizeof(float) * 3,
                targetCount,
                0.02f
            );

            bool sloppy = false;
            // if no improvement, try sloppy
            if (numOpt > simplified.size() * 0.9f) {
                if (lodLevel > 1) {
                    numOpt = meshopt_simplifySloppy(
                        simplified.data(),
                        simplified.data(),
                        static_cast<uint32_t>(simplified.size()),
                        vertices3f.data(),
                        static_cast<uint32_t>(vertexCount),
                        sizeof(float) * 3,
                        targetCount,
                        FLT_MAX,
                        nullptr
                    );
                    sloppy = true;
                    if (numOpt == simplified.size()) {
                        Logger::GetLogger()->warn("LOD{}: no further simplification possible.", lodLevel);
                        break;
                    }
                }
                else {
                    Logger::GetLogger()->warn("LOD{}: simplification not effective.", lodLevel);
                    break;
                }
            }
            simplified.resize(numOpt);

            // optimize for cache
            meshopt_optimizeVertexCache(
                simplified.data(),
                simplified.data(),
                static_cast<uint32_t>(numOpt),
                static_cast<uint32_t>(vertexCount)
            );

            currentIndexCount = numOpt;
            outLods.push_back(std::move(simplified));

            Logger::GetLogger()->info("LOD{} => {} indices {}",
                lodLevel, numOpt,
                sloppy ? "[sloppy]" : "");

            lodLevel++;
        }
    }

    void ModelLoader::CenterMeshes()
    {
        if (m_Objects.empty()) {
            return;
        }
        glm::vec3 sceneMin(FLT_MAX);
        glm::vec3 sceneMax(-FLT_MAX);

        // compute global bounding box
        for (auto& obj : m_Objects) {
            auto& mesh = obj.mesh;
            sceneMin = glm::min(sceneMin, mesh->minBounds);
            sceneMax = glm::max(sceneMax, mesh->maxBounds);
        }

        glm::vec3 center = 0.5f * (sceneMin + sceneMax);

        // shift every mesh
        for (auto& obj : m_Objects) {
            auto& mesh = obj.mesh;
            for (auto& p : mesh->positions) {
                p -= center;
            }
            mesh->minBounds -= center;
            mesh->maxBounds -= center;
            mesh->localCenter = 0.5f * (mesh->minBounds + mesh->maxBounds);
        }
    }

    glm::mat4 ModelLoader::AiToGlm(const aiMatrix4x4& m) const
    {
        glm::mat4 ret;
        ret[0][0] = m.a1; ret[1][0] = m.b1; ret[2][0] = m.c1; ret[3][0] = m.d1;
        ret[0][1] = m.a2; ret[1][1] = m.b2; ret[2][1] = m.c2; ret[3][1] = m.d2;
        ret[0][2] = m.a3; ret[1][2] = m.b3; ret[2][2] = m.c3; ret[3][2] = m.d3;
        ret[0][3] = m.a4; ret[1][3] = m.b4; ret[2][3] = m.c4; ret[3][3] = m.d4;

        return glm::transpose(ret);
    }

    std::string ModelLoader::GetModelPath(const std::string& modelName) const
    {
        auto it = m_ModelPaths.find(modelName);
        if (it != m_ModelPaths.end()) {
            return it->second;
        }
        Logger::GetLogger()->error("Unknown modelName '{}'. Check m_ModelPaths or add a new entry.", modelName);
        return "";
    }

} // namespace staticloader