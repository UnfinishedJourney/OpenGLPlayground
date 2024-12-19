#include "SceneGraph.h"

int SceneGraph::AddNode(int parent, const std::string& name) {
    NodeData nd;
    nd.parent = parent;
    nd.name = name;
    int node = (int)m_Nodes.size();
    m_Nodes.push_back(nd);
    if (parent >= 0) {
        int s = m_Nodes[parent].firstChild;
        if (s == -1) {
            m_Nodes[parent].firstChild = node;
            m_Nodes[node].lastSibling = node;
        }
        else {
            int dest = m_Nodes[s].lastSibling;
            if (dest <= -1) {
                for (dest = s; m_Nodes[dest].nextSibling != -1; dest = m_Nodes[dest].nextSibling);
            }
            m_Nodes[dest].nextSibling = node;
            m_Nodes[s].lastSibling = node;
        }
        m_Nodes[node].level = m_Nodes[parent].level + 1;
    }
    return node;
}

void SceneGraph::SetLocalTransform(int node, const glm::mat4& transform) {
    m_Nodes[node].localTransform = transform;
    // Mark changed:
    int lvl = m_Nodes[node].level;
    m_ChangedAtLevel[lvl].push_back(node);
}

void SceneGraph::SetNodeMesh(int node, int meshIndex) {
    m_Nodes[node].meshIndex = meshIndex;
}

void SceneGraph::SetNodeMaterial(int node, int materialIndex) {
    m_Nodes[node].materialIndex = materialIndex;
}

void SceneGraph::SetNodeBoundingVolumes(int node, const glm::vec3& minB, const glm::vec3& maxB, const glm::vec3& center, float radius) {
    m_Nodes[node].boundingBoxMin = minB;
    m_Nodes[node].boundingBoxMax = maxB;
    m_Nodes[node].boundingSphereCenter = center;
    m_Nodes[node].boundingSphereRadius = radius;
}

void SceneGraph::MarkAllChanged() {
    for (int i = 0; i < (int)m_Nodes.size(); i++) {
        int lvl = m_Nodes[i].level;
        m_ChangedAtLevel[lvl].push_back(i);
    }
}

void SceneGraph::RecalculateGlobalTransforms() {
    // Level 0 nodes:
    if (!m_ChangedAtLevel[0].empty()) {
        for (auto c : m_ChangedAtLevel[0]) {
            m_Nodes[c].globalTransform = m_Nodes[c].localTransform;
        }
        m_ChangedAtLevel[0].clear();
    }

    for (int i = 1; i < 16; i++) {
        for (auto c : m_ChangedAtLevel[i]) {
            int p = m_Nodes[c].parent;
            m_Nodes[c].globalTransform = m_Nodes[p].globalTransform * m_Nodes[c].localTransform;
        }
        m_ChangedAtLevel[i].clear();
    }
}