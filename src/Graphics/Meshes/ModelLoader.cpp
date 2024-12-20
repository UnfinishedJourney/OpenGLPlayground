#include "ModelLoader.h"

bool ModelLoader::LoadIntoSceneGraph(
    const Model& model,
    const MeshLayout& layout,
    SceneGraph& sceneGraph,
    std::vector<MeshInfo>& outMeshes,
    std::vector<std::string>& outMaterials)
{
    outMeshes = model.GetMeshesInfo();
    const auto& mats = model.GetMaterials();
    for (auto& m : mats) {
        outMaterials.push_back(m.name);
    }

    const auto& nodes = model.GetNodes();
    if (nodes.empty()) {
        return false;
    }

    std::vector<int> nodeMap(nodes.size(), -1);

    std::function<void(int, int)> processNode = [&](int idx, int parentSG) {
        const auto& n = nodes[idx];
        int sgNode = sceneGraph.AddNode(parentSG, n.name);
        nodeMap[idx] = sgNode;
        sceneGraph.SetLocalTransform(sgNode, n.localTransform);

        for (auto meshIdx : n.meshes) {
            int matIdx = outMeshes[meshIdx].materialIndex;
            sceneGraph.SetNodeMesh(sgNode, meshIdx);
            sceneGraph.SetNodeMaterial(sgNode, matIdx);

            auto& m = outMeshes[meshIdx].mesh;
            sceneGraph.SetNodeBoundingVolumes(sgNode, m->minBounds, m->maxBounds, m->localCenter, m->boundingSphereRadius);
        }

        for (auto c : n.children) {
            processNode(c, sgNode);
        }
        };

    processNode(0, -1);
    sceneGraph.MarkAllChanged();
    sceneGraph.RecalculateGlobalTransforms();
    return true;
}