#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <functional>

struct SceneGraphNode {
    int parentIndex_ = -1;
    std::vector<int> children_;
    std::string name_;

    glm::mat4 localTransform_ = glm::mat4(1.0f);
    glm::mat4 globalTransform_ = glm::mat4(1.0f);

    glm::vec3 boundingBoxMin_ = glm::vec3(0.0f);
    glm::vec3 boundingBoxMax_ = glm::vec3(0.0f);
    glm::vec3 boundingSphereCenter_ = glm::vec3(0.0f);
    float boundingSphereRadius_ = 0.0f;

    std::vector<int> meshIndices_;
    std::vector<int> materialIndices_;
};

class SceneGraph {
public:
    int AddNode(int parentIndex, const std::string& name);
    void SetLocalTransform(int nodeIndex, const glm::mat4& transform);
    void SetNodeBoundingVolumes(int nodeIndex,
        const glm::vec3& minBounds,
        const glm::vec3& maxBounds,
        const glm::vec3& sphereCenter,
        float sphereRadius);
    void AddMeshReference(int nodeIndex, int meshIndex, int materialIndex);
    void RecalculateGlobalTransforms();
    void TraverseGraph(const std::function<void(int nodeIndex)>& visitor) const;
    void TraverseGraphDFS(const std::function<bool(int nodeIndex)>& preVisitor) const;

    const std::vector<SceneGraphNode>& GetNodes() const { return nodes_; }

private:
    void computeGlobalTransformRecursive(int nodeIndex, const glm::mat4& parentGlobal);
    void dfsTraversal(int nodeIndex, const std::function<bool(int)>& preVisitor) const;

private:
    std::vector<SceneGraphNode> nodes_;
};