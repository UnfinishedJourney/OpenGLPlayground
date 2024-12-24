#include "SceneGraph.h"

int SceneGraph::AddNode(int parent, const std::string& name)
{
    SceneGraphNode node;
    node.parent = parent;
    node.name = name;

    int nodeIndex = (int)m_Nodes.size();
    m_Nodes.push_back(node);

    // Hook it under the parent if valid
    if (parent >= 0 && parent < (int)m_Nodes.size()) {
        m_Nodes[parent].children.push_back(nodeIndex);
    }
    return nodeIndex;
}

void SceneGraph::SetLocalTransform(int nodeIndex, const glm::mat4& transform)
{
    if (nodeIndex < 0 || nodeIndex >= (int)m_Nodes.size()) return;
    m_Nodes[nodeIndex].localTransform = transform;
}

void SceneGraph::SetNodeBoundingVolumes(int nodeIndex,
    const glm::vec3& minB,
    const glm::vec3& maxB,
    const glm::vec3& sphereCenter,
    float sphereRadius)
{
    if (nodeIndex < 0 || nodeIndex >= (int)m_Nodes.size()) return;
    m_Nodes[nodeIndex].boundingBoxMin = minB;
    m_Nodes[nodeIndex].boundingBoxMax = maxB;
    m_Nodes[nodeIndex].boundingSphereCenter = sphereCenter;
    m_Nodes[nodeIndex].boundingSphereRadius = sphereRadius;
}

void SceneGraph::AddMeshReference(int nodeIndex, int meshIndex, int materialIndex)
{
    if (nodeIndex < 0 || nodeIndex >= (int)m_Nodes.size()) return;
    m_Nodes[nodeIndex].meshIndices.push_back(meshIndex);
    m_Nodes[nodeIndex].materialIndices.push_back(materialIndex);
}

void SceneGraph::RecalculateGlobalTransforms()
{
    // For each root node (parent == -1), do a DFS
    for (int i = 0; i < (int)m_Nodes.size(); i++) {
        if (m_Nodes[i].parent == -1) {
            // This is a root
            computeGlobalTransformRecursive(i, glm::mat4(1.0f));
        }
    }
}

void SceneGraph::computeGlobalTransformRecursive(int nodeIndex, const glm::mat4& parentGlobal)
{
    auto& node = m_Nodes[nodeIndex];
    node.globalTransform = parentGlobal * node.localTransform;

    // Recurse children
    for (auto childIdx : node.children) {
        computeGlobalTransformRecursive(childIdx, node.globalTransform);
    }
}

void SceneGraph::TraverseGraph(std::function<void(int nodeIndex)> visitor)
{
    // Simple loop
    for (int i = 0; i < (int)m_Nodes.size(); i++) {
        visitor(i);
    }
}