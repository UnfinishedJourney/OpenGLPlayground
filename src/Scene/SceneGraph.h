#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>

class SceneGraph {
public:
    struct NodeData {
        int parent = -1;
        int firstChild = -1;
        int nextSibling = -1;
        int lastSibling = -1;
        int level = 0;
        glm::mat4 localTransform = glm::mat4(1.0f);
        glm::mat4 globalTransform = glm::mat4(1.0f);
        int meshIndex = -1;
        int materialIndex = -1;
        glm::vec3 boundingBoxMin = glm::vec3(0.0f);
        glm::vec3 boundingBoxMax = glm::vec3(0.0f);
        glm::vec3 boundingSphereCenter = glm::vec3(0.0f);
        float boundingSphereRadius = 0.0f;
        std::string name;
    };

    int AddNode(int parent, const std::string& name);
    void SetLocalTransform(int node, const glm::mat4& transform);
    void SetNodeMesh(int node, int meshIndex);
    void SetNodeMaterial(int node, int materialIndex);
    void SetNodeBoundingVolumes(int node, const glm::vec3& minB, const glm::vec3& maxB, const glm::vec3& center, float radius);
    void MarkAllChanged();
    void RecalculateGlobalTransforms();

    const std::vector<NodeData>& GetNodes() const { return m_Nodes; }

private:
    std::vector<NodeData> m_Nodes;
    std::vector<int> m_ChangedAtLevel[16];
};