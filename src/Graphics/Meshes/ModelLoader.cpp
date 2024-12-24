#include "ModelLoader.h"
#include "Graphics/Materials/Material.h"
#include "Resources/MaterialManager.h"
#include "Utilities/Logger.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <meshoptimizer.h>
#include <stdexcept>
#include <cfloat>
#include <filesystem>


bool ModelLoader::LoadModelIntoSceneGraph(const Model& model,
    SceneGraph& sceneGraph,
    std::vector<MeshInfo>& outMeshes,
    std::vector<std::string>& outMaterials)
{
    // 1) Copy mesh & material arrays
    outMeshes = model.GetMeshesInfo();
    auto mats = model.GetMaterials();
    for (auto& m : mats) {
        outMaterials.push_back(m.name);
    }

    // 2) Build scene graph nodes
    const auto& nodes = model.GetNodes();
    if (nodes.empty()) {
        return false;
    }

    // We'll store SG node indices
    std::vector<int> nodeMap(nodes.size(), -1);

    // Recursive lambda
    std::function<void(int, int)> processNode = [&](int idx, int parentSG) {
        const auto& n = nodes[idx];
        int sgNode = sceneGraph.AddNode(parentSG, n.name);
        nodeMap[idx] = sgNode;

        // local transform
        sceneGraph.SetLocalTransform(sgNode, n.localTransform);

        // For each mesh in the node
        for (auto meshIdx : n.meshes) {
            int materialIndex = outMeshes[meshIdx].materialIndex;
            sceneGraph.AddMeshReference(sgNode, meshIdx, materialIndex);

            // Also set bounding volumes for the node
            auto& meshPtr = outMeshes[meshIdx].mesh;
            if (meshPtr) {
                sceneGraph.SetNodeBoundingVolumes(sgNode,
                    meshPtr->minBounds,
                    meshPtr->maxBounds,
                    meshPtr->localCenter,
                    meshPtr->boundingSphereRadius
                );
            }
        }

        // Recurse children
        for (auto child : n.children) {
            processNode(child, sgNode);
        }
        };

    // Start from node 0 as root
    processNode(0, -1);

    // Recalc transforms
    sceneGraph.RecalculateGlobalTransforms();

    return true;
}

std::string ModelLoader::GetModelPath(const std::string& modelName) {
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


#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Implement GetModelPath as previously defined
std::string ModelLoader2::GetModelPath(const std::string& modelName) {
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

void ModelLoader2::CreateMaterial(const MaterialLayout& matLayout, const aiMaterial& aiMat)
{
    auto mat = std::make_shared<Material>();

    // Set Material Name
    aiString name;
    if (AI_SUCCESS == aiMat.Get(AI_MATKEY_NAME, name)) {
        mat->SetName(name.C_Str());
    }
    else {
        mat->SetName("UnnamedMaterial"); // Assign a default name or handle as needed
        Logger::GetLogger()->warn("Material without a name found.");
    }

    // Ambient Color
    aiColor3D ambientColor;
    if (matLayout.params.contains(MaterialParamType::Ambient) &&
        (AI_SUCCESS == aiMat.Get(AI_MATKEY_COLOR_AMBIENT, ambientColor))) {
        mat->SetParam(MaterialParamType::Ambient, glm::vec3(ambientColor.r, ambientColor.g, ambientColor.b));
    }
    else {
        // Set a default ambient color or handle as needed
        mat->SetParam(MaterialParamType::Ambient, glm::vec3(0.1f, 0.1f, 0.1f));
        Logger::GetLogger()->info("Ambient color not found. Using default.");
    }

    // Diffuse Color
    aiColor3D diffuseColor;
    if (matLayout.params.contains(MaterialParamType::Diffuse) &&
        (AI_SUCCESS == aiMat.Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor))) {
        mat->SetParam(MaterialParamType::Diffuse, glm::vec3(diffuseColor.r, diffuseColor.g, diffuseColor.b));
    }
    else {
        // Set a default diffuse color or handle as needed
        mat->SetParam(MaterialParamType::Diffuse, glm::vec3(0.5f, 0.5f, 0.5f));
        Logger::GetLogger()->info("Diffuse color not found. Using default.");
    }

    // Specular Color
    aiColor3D specularColor;
    if (matLayout.params.contains(MaterialParamType::Specular) &&
        (AI_SUCCESS == aiMat.Get(AI_MATKEY_COLOR_SPECULAR, specularColor))) {
        mat->SetParam(MaterialParamType::Specular, glm::vec3(specularColor.r, specularColor.g, specularColor.b));
    }
    else {
        // Set a default specular color or handle as needed
        mat->SetParam(MaterialParamType::Specular, glm::vec3(0.5f, 0.5f, 0.5f));
        Logger::GetLogger()->info("Specular color not found. Using default.");
    }

    // Shininess
    float shininess = 32.0f; // Default shininess
    if (matLayout.params.contains(MaterialParamType::Shininess)) {
        if (AI_SUCCESS != aiMat.Get(AI_MATKEY_SHININESS, shininess)) {
            Logger::GetLogger()->info("Shininess not found. Using default.");
        }
        // If retrieval failed, shininess remains default
        mat->SetParam(MaterialParamType::Shininess, shininess);
    }

    // TODO: Handle additional material parameters if needed

    // Add the material to the MaterialManager
    MaterialManager::GetInstance().AddMaterial(mat->GetName(), mat);

    // Store material name in ModelData
    m_ModelData.m_Materials.push_back(mat->GetName());
}

void ModelLoader2::EnsureUnknownMaterialExists()
{
    auto unknownMaterial = MaterialManager::GetInstance().GetMaterial("UnknownMaterial");
    if (!unknownMaterial)
    {
        // Define default parameters for UnknownMaterial
        glm::vec3 ambient(0.1f, 0.1f, 0.1f);
        glm::vec3 diffuse(0.5f, 0.5f, 0.5f);
        glm::vec3 specular(0.5f, 0.5f, 0.5f);
        float shininess = 32.0f;

        // Create the material
        unknownMaterial = MaterialManager::GetInstance().CreateMaterial("UnknownMaterial", ambient, diffuse, specular, shininess);

        // Add it to the MaterialManager
        MaterialManager::GetInstance().AddMaterial("UnknownMaterial", unknownMaterial);

        Logger::GetLogger()->info("Created default 'UnknownMaterial'.");
    }
}

bool ModelLoader2::LoadModel(
    const std::string& modelName,
    const MeshLayout& meshLayout,
    const MaterialLayout& matLayout)
{
    // Ensure "UnknownMaterial" exists
    EnsureUnknownMaterialExists();

    std::string modelPath = GetModelPath(modelName);
    if (modelPath.empty()) {
        Logger::GetLogger()->error("Model '{}' path not found.", modelName);
        return false;
    }

    Assimp::Importer importer;

    // Define desired Assimp post-processing flags
    unsigned int flags = aiProcess_JoinIdenticalVertices |
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_CalcTangentSpace |
        aiProcess_RemoveRedundantMaterials |
        aiProcess_FindInvalidData |
        aiProcess_OptimizeMeshes;

    const aiScene* scene = importer.ReadFile(modelPath, flags);

    if (!scene || !scene->HasMeshes()) {
        Logger::GetLogger()->error("Unable to load model: {}", modelPath);
        throw std::runtime_error("Unable to load model: " + modelPath);
    }

    // Load Materials
    for (unsigned int i = 0; i < scene->mNumMaterials; ++i) {
        const aiMaterial* aiMat = scene->mMaterials[i];
        CreateMaterial(matLayout, *aiMat);
    }

    // Initialize ModelData
    m_ModelData = ModelData();
    m_ModelData.m_MeshLayout = meshLayout;
    m_ModelData.m_MatLayout = matLayout;

    // Process Nodes starting from root
    ProcessNode(scene, scene->mRootNode, -1);

    // Optionally, compute bounding boxes or other model-wide data here

    Logger::GetLogger()->info("Model '{}' loaded successfully with {} materials and {} meshes.",
        modelName, scene->mNumMaterials, m_ModelData.m_Meshes.size());

    return true;
}

void ModelLoader2::ProcessNode(const aiScene* scene, const aiNode* node, int parentIndex)
{
    ModelNode currentNode;
    currentNode.parent_ = parentIndex;
    currentNode.level_ = (parentIndex == -1) ? 0 : m_Nodes[parentIndex].level_ + 1;

    // Convert aiMatrix4x4 to glm::mat4 (Assimp uses column-major order)
    aiMatrix4x4 aiTransform = node->mTransformation;
    glm::mat4 transform = glm::transpose(glm::make_mat4(&aiTransform.a1)); // Ensure correct orientation
    int currentTransformIndex = static_cast<int>(m_ModelData.m_LocalTransforms.size());
    m_ModelData.m_LocalTransforms.push_back(transform);
    currentNode.localTransform_ = currentTransformIndex;

    // Initialize sibling indices
    currentNode.firstChild_ = -1;
    currentNode.nextSibling_ = -1;
    currentNode.lastSibling_ = -1;

    // Handle parent-child relationships
    if (parentIndex != -1) {
        // Update the parent's last sibling
        if (m_Nodes[parentIndex].firstChild_ == -1) {
            m_Nodes[parentIndex].firstChild_ = static_cast<int>(m_Nodes.size());
        }

        if (m_Nodes[parentIndex].lastSibling_ != -1) {
            m_Nodes[m_Nodes[parentIndex].lastSibling_].nextSibling_ = static_cast<int>(m_Nodes.size());
        }

        m_Nodes[parentIndex].lastSibling_ = static_cast<int>(m_Nodes.size());
    }

    bool hasMesh = node->mNumMeshes > 0;

    if (hasMesh) {
        // Process all meshes referenced by this node
        for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
            const aiMesh* aiMesh = scene->mMeshes[node->mMeshes[i]];
            Mesh mesh = ProcessMesh(scene, aiMesh);
            m_ModelData.m_Meshes.push_back(std::make_shared<Mesh>(mesh));
            int meshIndex = static_cast<int>(m_ModelData.m_Meshes.size() - 1);
            currentNode.meshIndices.push_back(meshIndex);

            // Determine Material
            unsigned int materialIndex = aiMesh->mMaterialIndex;
            std::string materialName = "UnknownMaterial"; // Default material

            if (materialIndex < m_ModelData.m_Materials.size()) {
                materialName = m_ModelData.m_Materials[materialIndex];
            }
            else {
                Logger::GetLogger()->warn("Mesh {} references invalid material index {}. Assigning 'UnknownMaterial'.", i, materialIndex);
            }

            // Assign material to the mesh via MaterialManager
            auto material = MaterialManager::GetInstance().GetMaterial(materialName);
            if (!material) {
                Logger::GetLogger()->error("Material '{}' not found. Assigning 'UnknownMaterial'.", materialName);
                material = MaterialManager::GetInstance().GetMaterial("UnknownMaterial");
                if (!material) {
                    Logger::GetLogger()->error("'UnknownMaterial' is missing.");
                }
            }

            // Here you can link the mesh to the material in your SceneGraph or rendering pipeline
            // For example:
            // sceneGraph.AssignMaterialToMesh(meshIndex, material);
        }
    }
    else {
        // Node has no meshes; meshIndices remains empty
    }

    // Add current node to the node list
    int currentNodeIndex = static_cast<int>(m_Nodes.size());
    m_Nodes.push_back(currentNode);

    // Recursively process child nodes
    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        ProcessNode(scene, node->mChildren[i], currentNodeIndex);
    }
}

Mesh ModelLoader2::ProcessMesh(const aiScene* scene, const aiMesh* aiMesh)
{
    Mesh mesh;

    // Positions
    if (aiMesh->HasPositions()) {
        std::vector<glm::vec3> positions;
        positions.reserve(aiMesh->mNumVertices);
        for (unsigned int i = 0; i < aiMesh->mNumVertices; ++i) {
            positions.emplace_back(
                aiMesh->mVertices[i].x,
                aiMesh->mVertices[i].y,
                aiMesh->mVertices[i].z
            );
        }
        mesh.positions = positions;
    }

    // Normals
    if (aiMesh->HasNormals()) {
        std::vector<glm::vec3> normals;
        normals.reserve(aiMesh->mNumVertices);
        for (unsigned int i = 0; i < aiMesh->mNumVertices; ++i) {
            normals.emplace_back(
                aiMesh->mNormals[i].x,
                aiMesh->mNormals[i].y,
                aiMesh->mNormals[i].z
            );
        }
        mesh.normals = normals;
    }

    // Tangents and Bitangents
    if (aiMesh->HasTangentsAndBitangents()) {
        std::vector<glm::vec3> tangents;
        std::vector<glm::vec3> bitangents;
        tangents.reserve(aiMesh->mNumVertices);
        bitangents.reserve(aiMesh->mNumVertices);
        for (unsigned int i = 0; i < aiMesh->mNumVertices; ++i) {
            tangents.emplace_back(
                aiMesh->mTangents[i].x,
                aiMesh->mTangents[i].y,
                aiMesh->mTangents[i].z
            );
            bitangents.emplace_back(
                aiMesh->mBitangents[i].x,
                aiMesh->mBitangents[i].y,
                aiMesh->mBitangents[i].z
            );
        }
        mesh.tangents = tangents;
        mesh.bitangents = bitangents;
    }

    // Texture Coordinates
    for (unsigned int texCoordIndex = 0; texCoordIndex < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++texCoordIndex) {
        if (aiMesh->HasTextureCoords(texCoordIndex)) {
            std::vector<glm::vec2> uvs;
            uvs.reserve(aiMesh->mNumVertices);
            for (unsigned int i = 0; i < aiMesh->mNumVertices; ++i) {
                uvs.emplace_back(
                    aiMesh->mTextureCoords[texCoordIndex][i].x,
                    aiMesh->mTextureCoords[texCoordIndex][i].y
                );
            }
            // Assuming TextureType enumeration aligns with texture coordinate indices
            TextureType texType = static_cast<TextureType>(texCoordIndex); // Adjust as needed
            mesh.uvs[texType] = std::move(uvs);
        }
    }

    // Indices
    mesh.indices.reserve(aiMesh->mNumFaces * 3); // Assuming triangulated
    for (unsigned int i = 0; i < aiMesh->mNumFaces; ++i) {
        const aiFace& face = aiMesh->mFaces[i];
        if (face.mNumIndices != 3) {
            Logger::GetLogger()->warn("Non-triangular face detected. Skipping.");
            continue;
        }
        mesh.indices.emplace_back(face.mIndices[0]);
        mesh.indices.emplace_back(face.mIndices[1]);
        mesh.indices.emplace_back(face.mIndices[2]);
    }

    // Compute Bounding Box and Bounding Sphere
    if (aiMesh->HasPositions()) {
        glm::vec3 minBounds(FLT_MAX);
        glm::vec3 maxBounds(-FLT_MAX);
        if (std::holds_alternative<std::vector<glm::vec3>>(mesh.positions)) {
            const auto& positions = std::get<std::vector<glm::vec3>>(mesh.positions);
            for (const auto& pos : positions) {
                mesh.minBounds = glm::min(mesh.minBounds, pos);
                mesh.maxBounds = glm::max(mesh.maxBounds, pos);
            }
        }
        else {
            throw std::runtime_error("mesh.positions does not contain glm::vec3");
        }
        mesh.minBounds = minBounds;
        mesh.maxBounds = maxBounds;
        mesh.localCenter = (minBounds + maxBounds) / 2.0f;
        mesh.boundingSphereRadius = glm::length(maxBounds - mesh.localCenter);
    }

    // LODs can be handled here if necessary (e.g., based on mesh complexity)

    return mesh;
}

