#include "StaticModelLoader.h"
#include <meshoptimizer.h>
#include <filesystem>
#include <cfloat>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/gtc/matrix_transform.hpp>

namespace staticloader {


    ModelLoader::ModelLoader(float scaleFactor, std::unordered_map<aiTextureType, TextureType> aiToMyType)
        : m_FallbackMaterialCounter(0),
        m_ScaleFactor(scaleFactor),
        m_AiToMyType(aiToMyType)
    {
    }

    ModelLoader::~ModelLoader()
    {
    }

    bool ModelLoader::LoadStaticModel(const std::string& modelName,
        const MeshLayout& meshLayout,
        const MaterialLayout& matLayout,
        bool centerModel)
    {
        m_Objects.clear();
        m_Materials.clear();
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

        //maybe i don't need binormals at all
        if (meshLayout.hasTangents || meshLayout.hasBitangents) {
            importFlags |= aiProcess_CalcTangentSpace;
        }

        auto filePath = ModelLoader::GetModelPath(modelName);
        if (filePath == "")
        {
            return false;
        }

        const aiScene* scene = importer.ReadFile(filePath, importFlags);
        if (!scene || !scene->HasMeshes())
        {
            Logger::GetLogger()->error("Assimp failed to load model from '{}'.", filePath);
            return false;
        }

        std::string directory = std::filesystem::path(filePath).parent_path().string();
        LoadSceneMaterials(scene, matLayout, directory);

        std::vector<std::pair<aiNode*, glm::mat4>> stack;
        stack.push_back({ scene->mRootNode, glm::mat4(1.0f) });

        while (!stack.empty()) {
            auto [node, parentTransform] = stack.back();
            stack.pop_back();

            // Convert node's local transform to glm
            glm::mat4 localTransform = AiToGlm(node->mTransformation);
            glm::mat4 globalTransform = parentTransform * localTransform;

            // Process each mesh referenced by this node
            for (unsigned int i = 0; i < node->mNumMeshes; i++) {
                unsigned meshIndex = node->mMeshes[i];
                aiMesh* aimesh = scene->mMeshes[meshIndex];

                // Bake the node's global transform into the final vertex data
                std::shared_ptr<Mesh> newMesh = ProcessAssimpMesh(aimesh, meshLayout, globalTransform);

                // Find or create the material for this mesh
                int matIndex = aimesh->mMaterialIndex;
                std::shared_ptr<Material> matPtr;
                if (matIndex < 0 || matIndex >= static_cast<int>(m_Materials.size())) {
                    // If out of range, create a fallback material
                    auto fallbackMat = std::make_shared<Material>(matLayout);
                    fallbackMat->SetName("FallbackMat_" + std::to_string(++m_FallbackMaterialCounter));
                    MaterialManager::GetInstance().AddMaterial(fallbackMat);
                    m_Materials.push_back(fallbackMat);
                    matPtr = fallbackMat;
                }
                else {
                    matPtr = m_Materials[matIndex];
                }

                // Store the new mesh + material
                MeshInfo info;
                info.mesh = newMesh;
                info.materialIndex = matPtr->GetID();
                m_Objects.push_back(info);
            }

            // Push children with the updated transform
            for (unsigned int c = 0; c < node->mNumChildren; c++) {
                stack.push_back({ node->mChildren[c], globalTransform });
            }
        }

        return true;
    }

    void ModelLoader::LoadSceneMaterials(const aiScene* scene,
        const MaterialLayout& matLayout,
        const std::string& directory)
    {
        m_Materials.reserve(scene->mNumMaterials);
        for (unsigned int i = 0; i < scene->mNumMaterials; i++)
        {
            aiMaterial* aiMat = scene->mMaterials[i];
            std::shared_ptr<Material> mat = CreateMaterialForAssimpMat(aiMat, matLayout, directory);
            m_Materials.push_back(mat);
        }
    }

    std::shared_ptr<Material> ModelLoader::CreateMaterialForAssimpMat(const aiMaterial* aiMat,
        const MaterialLayout& matLayout,
        const std::string& directory)
    {
        aiString aiName;
        if (AI_SUCCESS != aiMat->Get(AI_MATKEY_NAME, aiName))
        {
            aiName = aiString("UnnamedMaterial" + std::to_string(m_UnnamedMaterialCounter++)); //maybe should rename it somehow to model name + counter, probably should check other names for uniqueness
        }

        std::string matName = aiName.C_Str();
        auto existingMat = MaterialManager::GetInstance().GetMaterialByName(matName);
        if (existingMat) {
            // Material already exists, reuse
            return existingMat;
        }

        auto mat = std::make_shared<Material>(matLayout);
        mat->SetName(matName);
        LoadMaterialProperties(aiMat, mat, matLayout);
        LoadMaterialTextures(aiMat, mat, matLayout, directory);

        MaterialManager::GetInstance().AddMaterial(mat);
        return mat;
    }


    void ModelLoader::LoadMaterialProperties(const aiMaterial* aiMat,
        std::shared_ptr<Material> mat,
        const MaterialLayout& matLayout)
    {
        // Example for Ambient
        if (matLayout.params.count(MaterialParamType::Ambient)) {
            aiColor3D color(0.2f, 0.2f, 0.2f);
            aiMat->Get(AI_MATKEY_COLOR_AMBIENT, color);
            mat->AssignToPackedParams(MaterialParamType::Ambient, glm::vec3(color.r, color.g, color.b));
        }
        // Similarly for Diffuse, Specular, Shininess, etc.
        if (matLayout.params.count(MaterialParamType::Diffuse)) {
            aiColor3D color(0.8f, 0.8f, 0.8f);
            aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
            mat->AssignToPackedParams(MaterialParamType::Diffuse, glm::vec3(color.r, color.g, color.b));
        }
        if (matLayout.params.count(MaterialParamType::Specular)) {
            aiColor3D color(0.0f, 0.0f, 0.0f);
            aiMat->Get(AI_MATKEY_COLOR_SPECULAR, color);
            mat->AssignToPackedParams(MaterialParamType::Specular, glm::vec3(color.r, color.g, color.b));
        }
        if (matLayout.params.count(MaterialParamType::Shininess)) {
            float shininessVal = 32.0f;
            aiMat->Get(AI_MATKEY_SHININESS, shininessVal);
            mat->AssignToPackedParams(MaterialParamType::Shininess, shininessVal);
        }
    }

    void ModelLoader::LoadMaterialTextures(const aiMaterial* aiMat,
        std::shared_ptr<Material> mat,
        const MaterialLayout& matLayout,
        const std::string& directory)
    {
        for (auto& [aiType, myType] : m_AiToMyType) {
            unsigned int texCount = aiMat->GetTextureCount(aiType);
            for (unsigned int i = 0; i < texCount; i++) {
                aiString str;
                if (aiMat->GetTexture(aiType, i, &str) == AI_SUCCESS) {
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
                    mat->SetTexture(myType, loadedTex);
                    Logger::GetLogger()->info("Texture '{}' loaded (type {}).", fullPath.string(), (int)myType);
                }
            }
        }
    }

    std::shared_ptr<Mesh> ModelLoader::ProcessAssimpMesh(const aiMesh* aimesh,
        const MeshLayout& meshLayout,
        const glm::mat4& transform)
    {
        auto newMesh = std::make_shared<Mesh>();

        // Positions
        if (meshLayout.hasPositions && aimesh->HasPositions()) {
            newMesh->positions.reserve(aimesh->mNumVertices);

            // Precompute scale+transform factor
            // We'll multiply each vertex by (transform * scaleFactor)
            for (unsigned v = 0; v < aimesh->mNumVertices; v++) {
                glm::vec4 originalPos(m_ScaleFactor * aimesh->mVertices[v].x,
                    m_ScaleFactor * aimesh->mVertices[v].y,
                    m_ScaleFactor * aimesh->mVertices[v].z,
                    1.0f);
                // apply transform & global scale
                glm::vec4 transformedPos = transform * (originalPos);

                glm::vec3 pos(transformedPos.x, transformedPos.y, transformedPos.z);
                newMesh->positions.push_back(pos);

                newMesh->minBounds = glm::min(newMesh->minBounds, pos);
                newMesh->maxBounds = glm::max(newMesh->maxBounds, pos);
            }
        }

        // Normals
        if (meshLayout.hasNormals && aimesh->HasNormals()) {
            newMesh->normals.reserve(aimesh->mNumVertices);

            // For correct normal transform, we use the inverse transpose of the 3x3 part
            glm::mat3 normalMat = glm::mat3(glm::transpose(glm::inverse(transform)));

            for (unsigned v = 0; v < aimesh->mNumVertices; v++) {
                glm::vec3 normal(aimesh->mNormals[v].x,
                    aimesh->mNormals[v].y,
                    aimesh->mNormals[v].z);
                normal = normalMat * normal;
                normal = glm::normalize(normal);

                newMesh->normals.push_back(normal);
            }
        }

        // Tangents / Bitangents
        if ((meshLayout.hasTangents || meshLayout.hasBitangents) &&
            aimesh->HasTangentsAndBitangents())
        {
            glm::mat3 tbMat = glm::mat3(glm::transpose(glm::inverse(transform)));

            if (meshLayout.hasTangents) {
                newMesh->tangents.reserve(aimesh->mNumVertices);
            }
            if (meshLayout.hasBitangents) {
                newMesh->bitangents.reserve(aimesh->mNumVertices);
            }

            for (unsigned v = 0; v < aimesh->mNumVertices; v++) {
                if (meshLayout.hasTangents) {
                    glm::vec3 t(aimesh->mTangents[v].x,
                        aimesh->mTangents[v].y,
                        aimesh->mTangents[v].z);
                    t = glm::normalize(tbMat * t);
                    newMesh->tangents.push_back(t);
                }

                if (meshLayout.hasBitangents) {
                    glm::vec3 b(aimesh->mBitangents[v].x,
                        aimesh->mBitangents[v].y,
                        aimesh->mBitangents[v].z);
                    b = glm::normalize(tbMat * b);
                    newMesh->bitangents.push_back(b);
                }
            }
        }

        // Texture coordinates
        // For simplicity, replicate the first UV set across each requested texture channel
        if (!meshLayout.textureTypes.empty() && aimesh->HasTextureCoords(0)) {
            std::vector<glm::vec2> uvSet(aimesh->mNumVertices);
            for (unsigned v = 0; v < aimesh->mNumVertices; v++) {
                uvSet[v] = glm::vec2(aimesh->mTextureCoords[0][v].x,
                    aimesh->mTextureCoords[0][v].y);
            }
            // Copy this uvSet to each requested texture channel
            for (auto texType : meshLayout.textureTypes) {
                newMesh->uvs[texType] = uvSet;
            }
        }

        // Gather all indices
        std::vector<uint32_t> srcIndices;
        srcIndices.reserve(aimesh->mNumFaces * 3);
        for (unsigned f = 0; f < aimesh->mNumFaces; f++) {
            const aiFace& face = aimesh->mFaces[f];
            // Typically each face is a triangle after aiProcess_Triangulate
            for (unsigned idx = 0; idx < face.mNumIndices; idx++) {
                srcIndices.push_back(face.mIndices[idx]);
            }
        }

        // Basic bounding data
        newMesh->localCenter = 0.5f * (newMesh->minBounds + newMesh->maxBounds);
        newMesh->boundingSphereRadius = glm::length(newMesh->maxBounds - newMesh->localCenter);

        // Prepare for meshoptimizer LOD generation
        std::vector<float> floatPositions;
        floatPositions.reserve(newMesh->positions.size() * 3);
        for (auto& pos : newMesh->positions) {
            floatPositions.push_back(pos.x);
            floatPositions.push_back(pos.y);
            floatPositions.push_back(pos.z);
        }

        // Generate multiple LOD index buffers
        std::vector<std::vector<uint32_t>> lodIndices;
        GenerateLODs(srcIndices, floatPositions, lodIndices);

        // Move the final indices/LODs into the Mesh
        newMesh->indices.clear();
        newMesh->lods.clear();

        for (auto& singleLOD : lodIndices) {
            MeshLOD lod;
            lod.indexOffset = static_cast<uint32_t>(newMesh->indices.size());
            lod.indexCount = static_cast<uint32_t>(singleLOD.size());

            newMesh->indices.insert(newMesh->indices.end(), singleLOD.begin(), singleLOD.end());
            newMesh->lods.push_back(lod);
        }

        return newMesh;
    }

    void ModelLoader::GenerateLODs(const std::vector<uint32_t>& srcIndices,
        const std::vector<float>& vertices3f,
        std::vector<std::vector<uint32_t>>& outLods) const
    {
        // If we have no data, return a single "LOD0" = original
        if (srcIndices.empty() || vertices3f.empty()) {
            outLods.emplace_back(srcIndices);
            return;
        }

        size_t vertexCount = vertices3f.size() / 3;
        size_t currentIndexCount = srcIndices.size();

        // LOD0 = full-res
        outLods.emplace_back(srcIndices);

        // Attempt to create LOD1..LOD7
        uint8_t lodLevel = 1;
        while (currentIndexCount > 1024 && lodLevel < 8) {
            size_t targetCount = currentIndexCount / 2;

            // Start from the previous LOD's indices
            const std::vector<uint32_t>& prevLOD = outLods.back();
            std::vector<uint32_t> simplified(prevLOD);

            // meshoptimizer simplify
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
            // If we didn't gain much reduction, try sloppy for subsequent LODs
            if (numOptIndices > simplified.size() * 0.9f) {
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

                    // If STILL no improvement, break
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

            // Optimize vertex cache
            meshopt_optimizeVertexCache(
                simplified.data(),
                simplified.data(),
                static_cast<uint32_t>(numOptIndices),
                vertexCount
            );

            // Accept LOD
            currentIndexCount = numOptIndices;
            outLods.emplace_back(simplified);

            Logger::GetLogger()->info("LOD{} => {} indices {}",
                lodLevel, numOptIndices,
                sloppy ? "[sloppy]" : "");
            lodLevel++;
        }
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
        auto it = m_ModelPaths.find(modelName);
        if (it != m_ModelPaths.end()) {
            return it->second;
        }
        else {
            Logger::GetLogger()->error("Unknown model name '{}'. Check your path registry.", modelName);
        }
        return "";
    }
}