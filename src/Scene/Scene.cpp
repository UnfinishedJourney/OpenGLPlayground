#include "Scene.h"

#include <glad/glad.h>
#include <stdexcept>
#include <glm/gtc/matrix_transform.hpp>

// Example struct for frame data UBO
struct FrameCommonData
{
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec4 cameraPos;
};

// Binding points, must match your shader
constexpr GLuint FRAME_DATA_BINDING_POINT = 0;
constexpr GLuint LIGHTS_DATA_BINDING_POINT = 1;
static const size_t MAX_LIGHTS = 32; // Example limit

Scene::Scene()
{
    // Default camera
    m_Camera = std::make_shared<Camera>();

    // Create the UBO for frame data
    m_FrameDataUBO = std::make_unique<UniformBuffer>(
        sizeof(FrameCommonData),
        FRAME_DATA_BINDING_POINT,
        GL_DYNAMIC_DRAW
    );

    // Create the Lights SSBO
    GLsizeiptr bufferSize = sizeof(glm::vec4) + MAX_LIGHTS * sizeof(LightData);
    uint32_t numLights = 0;
    m_LightsSSBO = std::make_unique<ShaderStorageBuffer>(
        LIGHTS_DATA_BINDING_POINT,
        bufferSize,
        GL_DYNAMIC_DRAW
    );
    // First 16 bytes = vec4 storing number of lights
    m_LightsSSBO->SetData(&numLights, sizeof(glm::vec4), 0);
    m_LightsSSBO->BindBase();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // LOD + Culling
    m_LODEvaluator = std::make_shared<LODEvaluator>();
    m_FrustumCuller = std::make_shared<FrustumCuller>();
}

Scene::~Scene()
{
    Clear();
}

void Scene::Clear()
{
    // Clear everything so we can load a new scene
    m_LightsData.clear();

    m_StaticBatchManager.Clear();
    m_SceneGraph = SceneGraph(); // re-init
    m_StaticObjects.clear();

    m_LoadedMeshes.clear();
    m_LoadedMaterials.clear();
    m_NodeToRenderObjects.clear();

    m_LastShader.clear();
    m_StaticBatchesDirty = true;
}

void Scene::SetCamera(const std::shared_ptr<Camera>& camera)
{
    m_Camera = camera;
}

void Scene::AddLight(const LightData& light)
{
    if (m_LightsData.size() >= MAX_LIGHTS) {
        Logger::GetLogger()->warn("Max number of lights ({}) reached!", MAX_LIGHTS);
        return;
    }
    m_LightsData.push_back(light);
    UpdateLightsData();
}

void Scene::UpdateLightsData()
{
    if (m_LightsData.size() > MAX_LIGHTS) {
        throw std::runtime_error("Too many lights in the scene!");
    }

    // The first vec4 in the SSBO is the number of lights
    uint32_t numLights = static_cast<uint32_t>(m_LightsData.size());
    m_LightsSSBO->SetData(&numLights, sizeof(glm::vec4), 0);

    // Followed by the array of LightData
    m_LightsSSBO->SetData(
        m_LightsData.data(),
        m_LightsData.size() * sizeof(LightData),
        sizeof(glm::vec4)
    );
}

void Scene::BindLightSSBO() const
{
    m_LightsSSBO->Bind();
}

void Scene::SetPostProcessingEffect(PostProcessingEffectType effect)
{
    m_PostProcessingEffect = effect;
}

PostProcessingEffectType Scene::GetPostProcessingEffect() const
{
    return m_PostProcessingEffect;
}

//------------------------------------------------------------------------------------
// 1) NEW STATIC-LOADER-BASED METHOD
//    Loads a model whose node hierarchy is baked into final vertex positions.
//    We store each resulting mesh as a "static" RenderObject in m_StaticObjects.
//------------------------------------------------------------------------------------
bool Scene::LoadStaticModelIntoScene(const std::string& modelName,
    const std::string& shaderName,
    float scaleFactor,
    std::unordered_map<aiTextureType, TextureType> aiToMyType)
{
    // 1) Get mesh + material layouts based on the shader
    auto& resourceManager = ResourceManager::GetInstance();
    auto [meshLayout, matLayout] = resourceManager.getLayoutsFromShader(shaderName);

    m_LastLayout = meshLayout;

    // 2) Create the new static loader
    staticloader::ModelLoader loader(scaleFactor, aiToMyType);

    // 3) Actually load the model (bakes transforms)
    bool success = loader.LoadStaticModel(modelName, meshLayout, matLayout, /*centerModel=*/true);
    if (!success)
    {
        Logger::GetLogger()->error("Failed to load static model '{}'.", modelName);
        return false;
    }

    // 4) Retrieve the final loaded objects (mesh + material pairs)
    const auto& loadedObjects = loader.GetLoadedObjects();
    const auto& loadedMaterials = loader.GetLoadedMaterials();

    // 5) For each loaded object, create a RenderObject with identity transform
    //    (since the transforms are already baked into the mesh positions)
    for (auto& obj : loadedObjects)
    {
        // The Material pointer from the loader

        auto matPtr = MaterialManager::GetInstance().GetMaterialByID(obj.materialIndex);

        // In your engine, you might store the material name for referencing at render time
        std::string materialName = matPtr ? matPtr->GetName() : "UnnamedMaterial";

        // Keep track of this material name (for debugging or listing)
        m_LoadedMaterials.push_back(materialName);

        // Create a minimal transform with identity (the mesh is already in final space)
        auto transform = std::make_shared<Transform>();
        transform->SetModelMatrix(glm::mat4(1.0f));

        // Build a RenderObject
        RenderObject ro(
            obj.mesh,         // the baked mesh
            meshLayout,       // layout from the chosen shader
            materialName,     // material name
            shaderName,       // which shader to use at render
            transform
        );

        // Push into our “static” container
        m_StaticObjects.push_back(ro);
    }

    // We might not strictly need to store the layouts here,
    // but if you do, store them so you can reuse them later
    m_LastShader = shaderName;

    // Mark that we need to rebuild static batches
    m_StaticBatchesDirty = true;

    Logger::GetLogger()->info("Loaded static model '{}' => {} sub-meshes, {} materials",
        modelName,
        loadedObjects.size(),
        loadedMaterials.size());
    return true;
}

//------------------------------------------------------------------------------------
// 2) OLD "HIERARCHICAL" LOADING METHOD
//    Uses the original ModelLoader that populates the SceneGraph, etc.
//    If you do NOT need the SceneGraph, you can remove this method entirely.
//------------------------------------------------------------------------------------
bool Scene::LoadModelIntoScene(const std::string& modelName,
    const std::string& defaultShaderName,
    const std::string& defaultMaterialName,
    float scaleFactor,
    std::unordered_map<aiTextureType, TextureType> aiToMyType)
{
    auto& resourceManager = ResourceManager::GetInstance();

    // The ResourceManager knows how to get the correct layouts from a shader name
    auto [meshLayout, matLayout] = resourceManager.getLayoutsFromShader(defaultShaderName);

    m_LastLayout = meshLayout;
    // The old ModelLoader that populates a SceneGraph
    ModelLoader loader(scaleFactor, aiToMyType);

    // Build path from the old ModelLoader’s registry
    std::string modelPath = ModelLoader::GetModelPath(modelName);
    if (modelPath.empty())
    {
        Logger::GetLogger()->error("Unknown model name '{}'. Check your path registry.", modelName);
        return false;
    }

    // Load with hierarchical approach (SceneGraph will be populated)
    if (!loader.LoadModel(modelPath, meshLayout, matLayout, m_SceneGraph, true))
    {
        Logger::GetLogger()->error("Failed to load model '{}'.", modelName);
        return false;
    }

    // Grab the final loaded data
    const auto& data = loader.GetModelData();

    // Transfer mesh data to our local vector (these indices tie to SceneGraph node->mesh references)
    for (auto& md : data.meshesData)
    {
        MeshInfo info;
        info.mesh = md.mesh;
        // We do not necessarily know the material index here, so set 0 or do a lookup
        info.materialIndex = 0;
        m_LoadedMeshes.push_back(info);
    }

    // Keep track of created materials
    for (auto& matName : data.createdMaterials)
    {
        m_LoadedMaterials.push_back(matName);
    }

    // Keep the last used shader
    m_LastShader = defaultShaderName;

    // We’ll rebuild static batches only if you want the SceneGraph objects batched,
    // but typically these might be “dynamic.” Up to you:
    m_StaticBatchesDirty = true;

    // Make sure our node->RenderObject array is large enough to hold the new nodes
    m_NodeToRenderObjects.resize(m_SceneGraph.GetNodes().size());

    return true;
}

//------------------------------------------------------------------------------------
// Batching: We combine geometry with the same material/shader into bigger GPU buffers
//           for performance.  We do so for either the SceneGraph-based objects or
//           the new “static” objects. Adjust as needed.
//------------------------------------------------------------------------------------
void Scene::BuildStaticBatchesIfNeeded()
{
    if (!m_StaticBatchesDirty) {
        return;
    }
    m_StaticBatchesDirty = false;

    // 1) Clear old references
    m_StaticBatchManager.Clear();

    // 2) If you still want to handle SceneGraph-based objects, recalc transforms:
    m_SceneGraph.RecalculateGlobalTransforms();

    // Rebuild the node->RenderObjects array
    auto& nodes = m_SceneGraph.GetNodes();
    if (m_NodeToRenderObjects.size() < nodes.size()) {
        m_NodeToRenderObjects.resize(nodes.size());
    }
    for (auto& roVec : m_NodeToRenderObjects) {
        roVec.clear();
    }

    // For each node in the SceneGraph, create a RenderObject referencing the mesh
    for (size_t i = 0; i < nodes.size(); i++)
    {
        const auto& node = nodes[i];
        for (size_t k = 0; k < node.meshIndices.size(); k++)
        {
            int meshIdx = node.meshIndices[k];
            int matIdx = node.materialIndices[k];

            if (meshIdx < 0 || meshIdx >= (int)m_LoadedMeshes.size()) {
                Logger::GetLogger()->error("Invalid mesh index {} in node {}", meshIdx, i);
                continue;
            }
            if (matIdx < 0 || matIdx >= (int)m_LoadedMaterials.size()) {
                Logger::GetLogger()->warn("Invalid material index {} in node {}", matIdx, i);
                matIdx = 0; // fallback
            }
            auto& meshInfo = m_LoadedMeshes[meshIdx];
            auto meshPtr = meshInfo.mesh;
            std::string matName = m_LoadedMaterials[matIdx];

            // Create a transform based on the node’s global transform
            auto transform = std::make_shared<Transform>();
            transform->SetModelMatrix(node.globalTransform);

            // Build the RenderObject
            auto ro = std::make_shared<RenderObject>(
                meshPtr,
                m_LastLayout, // or m_MeshLayout if you store it
                matName,
                m_LastShader,
                transform
            );
            // Add to node->RenderObjects
            m_NodeToRenderObjects[i].push_back(ro);

            // Also add to batch manager
            m_StaticBatchManager.AddRenderObject(ro);
        }
    }

    // 3) Also batch the new “static” objects (the ones loaded with transforms baked in)
    for (auto& ro : m_StaticObjects)
    {
        // We wrap it in a shared_ptr because the BatchManager interface expects that
        auto roPtr = std::make_shared<RenderObject>(ro);
        m_StaticBatchManager.AddRenderObject(roPtr);
    }

    // 4) Let the BatchManager build final GPU buffers
    m_StaticBatchManager.BuildBatches();
}

const std::vector<std::shared_ptr<Batch>>& Scene::GetStaticBatches() const
{
    return m_StaticBatchManager.GetBatches();
}

//------------------------------------------------------------------------------------
// Per-frame uniform updates
//------------------------------------------------------------------------------------
void Scene::UpdateFrameDataUBO() const
{
    FrameCommonData data{};
    if (m_Camera)
    {
        data.view = m_Camera->GetViewMatrix();
        data.proj = m_Camera->GetProjectionMatrix();
        data.cameraPos = glm::vec4(m_Camera->GetPosition(), 1.f);
    }
    else
    {
        data.view = glm::mat4(1.f);
        data.proj = glm::mat4(1.f);
        data.cameraPos = glm::vec4(0.f, 0.f, 0.f, 1.f);
    }
    // Update the uniform buffer
    m_FrameDataUBO->SetData(&data, sizeof(FrameCommonData));
}

void Scene::BindFrameDataUBO() const
{
    m_FrameDataUBO->Bind();
}

//------------------------------------------------------------------------------------
// Culling & LOD
//------------------------------------------------------------------------------------
void Scene::CullAndLODUpdate()
{
    if (!m_Camera)
        return;

    // 1) Build the view-projection matrix
    glm::mat4 VP = m_Camera->GetProjectionMatrix() * m_Camera->GetViewMatrix();
    // Extract planes for frustum culling
    m_FrustumCuller->ExtractFrustumPlanes(VP);

    // 2) LOD evaluation
    //    We pass each RenderObject to LOD logic. If you do per-object culling or bounding volumes,
    //    you can do that here or inside the batch manager. For a typical approach:
    m_StaticBatchManager.UpdateLODs(m_Camera, *m_LODEvaluator);

    // If you want to do hierarchical culling with the SceneGraph, you can do a DFS that checks
    // bounding volumes at each node.  The new “static” objects do not have a node hierarchy,
    // so you can do bounding checks individually if desired.
}