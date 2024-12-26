#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <functional>

struct SceneGraphNode
{
    int parent = -1;
    std::vector<int> children;
    std::string name;

    glm::mat4 localTransform = glm::mat4(1.0f);
    glm::mat4 globalTransform = glm::mat4(1.0f);

    // For culling
    glm::vec3 boundingBoxMin = glm::vec3(0.0f);
    glm::vec3 boundingBoxMax = glm::vec3(0.0f);
    glm::vec3 boundingSphereCenter = glm::vec3(0.0f);
    float boundingSphereRadius = 0.0f;

    // If a node can hold multiple meshes
    std::vector<int> meshIndices;
    std::vector<int> materialIndices;
};

class SceneGraph
{
public:
    int AddNode(int parent, const std::string& name);
    void SetLocalTransform(int nodeIndex, const glm::mat4& transform);
    void SetNodeBoundingVolumes(int nodeIndex,
        const glm::vec3& minB,
        const glm::vec3& maxB,
        const glm::vec3& sphereCenter,
        float sphereRadius);
    void AddMeshReference(int nodeIndex, int meshIndex, int materialIndex);

    void RecalculateGlobalTransforms();

    // Traversal
    void TraverseGraph(std::function<void(int nodeIndex)> visitor);
    void TraverseGraphDFS(std::function<bool(int nodeIndex)> preVisitor);

    const std::vector<SceneGraphNode>& GetNodes() const { return m_Nodes; }

private:
    void computeGlobalTransformRecursive(int nodeIndex, const glm::mat4& parentGlobal);
    void dfsCulling(int nodeIndex, std::function<bool(int)> preVisitor);

private:
    std::vector<SceneGraphNode> m_Nodes;
};