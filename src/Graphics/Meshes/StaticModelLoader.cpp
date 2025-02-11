#include "StaticModelLoader.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <filesystem>
#include <cfloat>          // For FLT_MAX
#include <meshoptimizer.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "Graphics/Materials/MaterialManager.h"
#include "Graphics/Textures/TextureManager.h"
#include "Utilities/Logger.h"

namespace staticloader {

    // ––– Constructor –––
    ModelLoader::ModelLoader(float scaleFactor,
        std::unordered_map<aiTextureType, TextureType> aiToMyType,
        uint8_t maxLODs)
        : scaleFactor_(scaleFactor),
        aiToMyType_(std::move(aiToMyType)),
        maxLODs_(maxLODs)
    {}

    // ––– LoadStaticModel –––
    bool ModelLoader::LoadStaticModel(const std::string& modelName,
        const MeshLayout& meshLayout,
        const MaterialLayout& matLayout,
        bool centerModel)
    {
        // Clear previous data.
        objects_.clear();
        materialIDs_.clear();
        fallbackMaterialCounter_ = 0;
        unnamedMaterialCounter_ = 0;

        // Configure Assimp importer.
        Assimp::Importer importer;
        unsigned importFlags = aiProcess_JoinIdenticalVertices |
            aiProcess_Triangulate |
            aiProcess_RemoveRedundantMaterials |
            aiProcess_FindInvalidData |
            aiProcess_OptimizeMeshes; // Merges small meshes.
        if (meshLayout.hasNormals) {
            importFlags |= aiProcess_GenSmoothNormals;
        }
        if (meshLayout.hasTangents || meshLayout.hasBitangents) {
            importFlags |= aiProcess_CalcTangentSpace;
        }

        // Look up file path.
        std::string filePath = GetModelPath(modelName);
        if (filePath.empty()) {
            Logger::GetLogger()->error("ModelLoader: No path for model '{}'.", modelName);
            return false;
        }

        // Load the scene.
        const aiScene* scene = importer.ReadFile(filePath, importFlags);
        if (!scene || !scene->HasMeshes()) {
            Logger::GetLogger()->error("ModelLoader: Failed to load '{}'.", filePath);
            return false;
        }

        // Determine texture directory.
        std::string directory = std::filesystem::path(filePath).parent_path().string();

        // 1) Load scene materials.
        LoadSceneMaterials(scene, matLayout, directory);

        // 2) Traverse the scene node hierarchy (iteratively).
        std::vector<std::pair<aiNode*, glm::mat4>> stack;
        stack.push_back({ scene->mRootNode, glm::mat4(1.0f) });
        while (!stack.empty()) {
            auto [node, parentXform] = stack.back();
            stack.pop_back();

            glm::mat4 local = AiToGlm(node->mTransformation);
            glm::mat4 global = parentXform * local;

            // Process each mesh in this node.
            for (unsigned i = 0; i < node->mNumMeshes; i++) {
                unsigned meshIndex = node->mMeshes[i];
                aiMesh* aimesh = scene->mMeshes[meshIndex];

                // Bake the transformation into the mesh.
                auto newMesh = ProcessAssimpMesh(aimesh, meshLayout, global);

                // Determine the material ID.
                std::size_t matID = 0;
                std::size_t matIndex = aimesh->mMaterialIndex;
                if (matIndex >= materialIDs_.size()) {
                    // Use a fallback material.
                    auto fallbackMat = std::make_unique<Graphics::Material>(matLayout);
                    fallbackMat->SetName("FallbackMat_" + std::to_string(++fallbackMaterialCounter_));
                    auto idOpt = Graphics::MaterialManager::GetInstance().AddMaterial(std::move(fallbackMat));
                    if (!idOpt.has_value()) {
                        Logger::GetLogger()->error("ModelLoader: Failed to create fallback material.");
                        continue;
                    }
                    materialIDs_.push_back(idOpt.value());
                    matID = idOpt.value();
                }
                else {
                    matID = materialIDs_[matIndex];
                }

                // Create MeshInfo and store.
                MeshInfo mi;
                mi.mesh = newMesh;
                mi.materialIndex = matID;
                objects_.push_back(mi);
            }

            // Push children nodes.
            for (unsigned c = 0; c < node->mNumChildren; c++) {
                stack.push_back({ node->mChildren[c], global });
            }
        }

        // 3) Optionally recenter geometry.
        if (centerModel) {
            CenterMeshes();
        }

        Logger::GetLogger()->info("ModelLoader: Loaded '{}' => {} meshes, {} materials.",
            modelName, objects_.size(), materialIDs_.size());
        return true;
    }

    // ––– LoadSceneMaterials –––
    void ModelLoader::LoadSceneMaterials(const aiScene* scene,
        const MaterialLayout& matLayout,
        const std::string& directory)
    {
        materialIDs_.reserve(scene->mNumMaterials);
        for (unsigned i = 0; i < scene->mNumMaterials; i++) {
            aiMaterial* aimat = scene->mMaterials[i];
            auto matID = CreateMaterialForAssimpMat(aimat, matLayout, directory);
            materialIDs_.push_back(matID);
        }
    }

    // ––– CreateMaterialForAssimpMat –––
    std::size_t ModelLoader::CreateMaterialForAssimpMat(const aiMaterial* aiMat,
        const MaterialLayout& matLayout,
        const std::string& directory)
    {
        aiString aiName;
        if (AI_SUCCESS != aiMat->Get(AI_MATKEY_NAME, aiName)) {
            aiName = aiString(("UnnamedMat_" + std::to_string(unnamedMaterialCounter_++)).c_str());
        }
        std::string matName(aiName.C_Str());

        // Reuse an existing material if one exists.
        auto existingMatID = Graphics::MaterialManager::GetInstance().GetMaterialIDByName(matName);
        if (existingMatID.has_value()) {
            return existingMatID.value();
        }

        auto material = std::make_unique<Graphics::Material>(matLayout);
        material->SetName(matName);

        // Load material properties (colors, floats, etc.).
        LoadMaterialProperties(aiMat, material, matLayout);

        // Load texture references.
        LoadMaterialTextures(aiMat, material, matLayout, directory);

        // Register the new material.
        auto idOpt = Graphics::MaterialManager::GetInstance().AddMaterial(std::move(material));
        if (!idOpt.has_value()) {
            Logger::GetLogger()->error("ModelLoader: Failed to add material '{}'.", matName);
            return 0;
        }
        return idOpt.value();
    }

    // ––– LoadMaterialProperties –––
    void ModelLoader::LoadMaterialProperties(const aiMaterial* aiMat,
        const std::unique_ptr<Graphics::Material>& mat,
        const MaterialLayout& matLayout)
    {
        // Note: Use the layout’s helper methods (e.g. HasParam) instead of directly accessing the bitset.
        if (matLayout.HasParam(MaterialParamType::Ambient)) {
            aiColor3D color(0.2f);
            aiMat->Get(AI_MATKEY_COLOR_AMBIENT, color);
            mat->AssignToPackedParams(MaterialParamType::Ambient,
                glm::vec3(color.r, color.g, color.b));
        }
        if (matLayout.HasParam(MaterialParamType::Diffuse)) {
            aiColor3D color(0.8f);
            aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
            mat->AssignToPackedParams(MaterialParamType::Diffuse,
                glm::vec3(color.r, color.g, color.b));
        }
        if (matLayout.HasParam(MaterialParamType::Specular)) {
            aiColor3D color(0.0f);
            aiMat->Get(AI_MATKEY_COLOR_SPECULAR, color);
            mat->AssignToPackedParams(MaterialParamType::Specular,
                glm::vec3(color.r, color.g, color.b));
        }
        if (matLayout.HasParam(MaterialParamType::Shininess)) {
            float shininess = 32.0f;
            aiMat->Get(AI_MATKEY_SHININESS, shininess);
            mat->AssignToPackedParams(MaterialParamType::Shininess, shininess);
        }
    }

    // ––– LoadMaterialTextures –––
    void ModelLoader::LoadMaterialTextures(const aiMaterial* aiMat,
        const std::unique_ptr<Graphics::Material>& mat,
        const MaterialLayout& matLayout,
        const std::string& directory)
    {
        // For each mapping from Assimp texture type to your texture type.
        for (auto& [aiType, myType] : aiToMyType_) {
            unsigned count = aiMat->GetTextureCount(aiType);
            for (unsigned i = 0; i < count; i++) {
                aiString texPath;
                if (aiMat->GetTexture(aiType, i, &texPath) == AI_SUCCESS) {
                    // Build the full texture path.
                    std::filesystem::path rel(texPath.C_Str());
                    rel = rel.lexically_normal();
                    std::filesystem::path full = std::filesystem::path(directory) / rel;
                    full = full.lexically_normal();

                    auto textureName = rel.filename().string();
                    auto loadedTex = Graphics::TextureManager::GetInstance().LoadTexture(textureName, full.string());
                    if (!loadedTex) {
                        Logger::GetLogger()->error("Failed to load texture '{}' for type={}.",
                            full.string(), static_cast<int>(myType));
                        continue;
                    }
                    mat->SetTexture(myType, loadedTex);
                    Logger::GetLogger()->info("Loaded texture '{}' (type={}).", full.string(), static_cast<int>(myType));
                }
            }
        }
    }

    // ––– ProcessAssimpMesh –––
    std::shared_ptr<Mesh> ModelLoader::ProcessAssimpMesh(const aiMesh* aimesh,
        const MeshLayout& meshLayout,
        const glm::mat4& transform)
    {
        auto mesh = std::make_shared<Mesh>();

        // Process positions.
        if (meshLayout.hasPositions && aimesh->HasPositions()) {
            mesh->positions.reserve(aimesh->mNumVertices);
            glm::vec4 tmp;
            for (unsigned v = 0; v < aimesh->mNumVertices; v++) {
                tmp.x = scaleFactor_ * aimesh->mVertices[v].x;
                tmp.y = scaleFactor_ * aimesh->mVertices[v].y;
                tmp.z = scaleFactor_ * aimesh->mVertices[v].z;
                tmp.w = 1.0f;
                glm::vec4 worldPos = transform * tmp;
                glm::vec3 finalPos(worldPos);
                mesh->positions.push_back(finalPos);

                // Update bounding box.
                mesh->minBounds = glm::min(mesh->minBounds, finalPos);
                mesh->maxBounds = glm::max(mesh->maxBounds, finalPos);
            }
        }

        // Process normals.
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

        // Process tangents and bitangents.
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

        // Process UVs.
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

        // Process indices.
        std::vector<uint32_t> srcIndices;
        srcIndices.reserve(aimesh->mNumFaces * 3);
        for (unsigned f = 0; f < aimesh->mNumFaces; f++) {
            const aiFace& face = aimesh->mFaces[f];
            for (unsigned idx = 0; idx < face.mNumIndices; idx++) {
                srcIndices.push_back(face.mIndices[idx]);
            }
        }

        // Compute bounding volumes.
        mesh->localCenter = 0.5f * (mesh->minBounds + mesh->maxBounds);
        mesh->boundingSphereRadius = glm::length(mesh->maxBounds - mesh->localCenter);

        // Prepare vertex positions for LOD generation.
        std::vector<float> floatPositions;
        floatPositions.reserve(mesh->positions.size() * 3);
        for (const auto& pos : mesh->positions) {
            floatPositions.push_back(pos.x);
            floatPositions.push_back(pos.y);
            floatPositions.push_back(pos.z);
        }

        // Generate LODs.
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

    // ––– GenerateLODs –––
    void ModelLoader::GenerateLODs(std::vector<uint32_t> srcIndices,
        const std::vector<float>& vertices3f,
        std::vector<std::vector<uint32_t>>& outLods) const
    {
        if (srcIndices.empty() || vertices3f.empty()) {
            outLods.push_back(std::move(srcIndices));
            return;
        }

        const size_t vertexCount = vertices3f.size() / 3;
        size_t currentIndexCount = srcIndices.size();

        // LOD0: the original indices.
        outLods.push_back(srcIndices);

        size_t lodLevel = 1;
        while (lodLevel < maxLODs_ && currentIndexCount > 1024) {
            size_t targetCount = currentIndexCount / 2;
            const auto& prevLOD = outLods.back();
            std::vector<uint32_t> simplified(prevLOD);

            // Simplify the mesh.
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
            // If no significant improvement is achieved, try a “sloppy” simplification.
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

            // Optimize for vertex cache.
            meshopt_optimizeVertexCache(
                simplified.data(),
                simplified.data(),
                static_cast<uint32_t>(numOpt),
                static_cast<uint32_t>(vertexCount)
            );

            currentIndexCount = numOpt;
            outLods.push_back(std::move(simplified));

            Logger::GetLogger()->info("LOD{} => {} indices {}", lodLevel, numOpt, sloppy ? "[sloppy]" : "");
            lodLevel++;
        }
    }

    // ––– CenterMeshes –––
    void ModelLoader::CenterMeshes()
    {
        if (objects_.empty()) {
            return;
        }
        glm::vec3 sceneMin(FLT_MAX);
        glm::vec3 sceneMax(-FLT_MAX);
        // Compute global bounding box.
        for (auto& obj : objects_) {
            auto& mesh = obj.mesh;
            sceneMin = glm::min(sceneMin, mesh->minBounds);
            sceneMax = glm::max(sceneMax, mesh->maxBounds);
        }
        glm::vec3 center = 0.5f * (sceneMin + sceneMax);
        // Shift every mesh.
        for (auto& obj : objects_) {
            auto& mesh = obj.mesh;
            for (auto& p : mesh->positions) {
                p -= center;
            }
            mesh->minBounds -= center;
            mesh->maxBounds -= center;
            mesh->localCenter = 0.5f * (mesh->minBounds + mesh->maxBounds);
        }
    }

    // ––– AiToGlm –––
    glm::mat4 ModelLoader::AiToGlm(const aiMatrix4x4& m) const
    {
        glm::mat4 ret;
        ret[0][0] = m.a1; ret[1][0] = m.b1; ret[2][0] = m.c1; ret[3][0] = m.d1;
        ret[0][1] = m.a2; ret[1][1] = m.b2; ret[2][1] = m.c2; ret[3][1] = m.d2;
        ret[0][2] = m.a3; ret[1][2] = m.b3; ret[2][2] = m.c3; ret[3][2] = m.d3;
        ret[0][3] = m.a4; ret[1][3] = m.b4; ret[2][3] = m.c4; ret[3][3] = m.d4;
        return glm::transpose(ret);
    }

    // ––– GetModelPath –––
    std::string ModelLoader::GetModelPath(const std::string& modelName) const
    {
        auto it = modelPaths_.find(modelName);
        if (it != modelPaths_.end()) {
            return it->second;
        }
        Logger::GetLogger()->error("Unknown modelName '{}'. Check modelPaths_ or add a new entry.", modelName);
        return "";
    }

} // namespace staticloader