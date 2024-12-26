#include "SceneGraph.h"

int SceneGraph::AddNode(int parent, const std::string& name)
{
    SceneGraphNode node;
    node.parent = parent;
    node.name = name;

    int nodeIndex = (int)m_Nodes.size();
    m_Nodes.push_back(node);

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
    for (int i = 0; i < (int)m_Nodes.size(); i++) {
        if (m_Nodes[i].parent == -1) {
            computeGlobalTransformRecursive(i, glm::mat4(1.0f));
        }
    }
}

void SceneGraph::computeGlobalTransformRecursive(int nodeIndex, const glm::mat4& parentGlobal)
{
    auto& node = m_Nodes[nodeIndex];
    node.globalTransform = parentGlobal * node.localTransform;

    for (auto childIdx : node.children) {
        computeGlobalTransformRecursive(childIdx, node.globalTransform);
    }
}

void SceneGraph::TraverseGraph(std::function<void(int nodeIndex)> visitor)
{
    for (int i = 0; i < (int)m_Nodes.size(); i++) {
        visitor(i);
    }
}

// DFS with early-out. preVisitor returns false => skip children.
void SceneGraph::TraverseGraphDFS(std::function<bool(int)> preVisitor)
{
    for (int i = 0; i < (int)m_Nodes.size(); i++) {
        if (m_Nodes[i].parent == -1) {
            dfsCulling(i, preVisitor);
        }
    }
}

void SceneGraph::dfsCulling(int nodeIndex, std::function<bool(int)> preVisitor)
{
    if (!preVisitor(nodeIndex)) {
        // If preVisitor says "false," skip children
        return;
    }
    for (auto child : m_Nodes[nodeIndex].children) {
        dfsCulling(child, preVisitor);
    }
}