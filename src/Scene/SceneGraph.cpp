#include "SceneGraph.h"
#include "Utilities/Logger.h"  
#include <stdexcept>

int SceneGraph::AddNode(int parentIndex, const std::string& name) {
    SceneGraphNode node;
    node.parentIndex_ = parentIndex;
    node.name_ = name;

    int nodeIndex = static_cast<int>(nodes_.size());
    nodes_.push_back(node);

    if (parentIndex >= 0) {
        if (parentIndex >= static_cast<int>(nodes_.size())) {
            Logger::GetLogger()->error("AddNode: Invalid parent index {} for node '{}'.", parentIndex, name);
            throw std::out_of_range("Invalid parent index");
        }
        nodes_[parentIndex].children_.push_back(nodeIndex);
    }
    Logger::GetLogger()->info("Added node '{}' (index={})", name, nodeIndex);
    return nodeIndex;
}

void SceneGraph::SetLocalTransform(int nodeIndex, const glm::mat4& transform) {
    if (nodeIndex < 0 || nodeIndex >= static_cast<int>(nodes_.size())) {
        Logger::GetLogger()->error("SetLocalTransform: Invalid node index {}.", nodeIndex);
        throw std::out_of_range("Invalid node index in SetLocalTransform");
    }
    nodes_[nodeIndex].localTransform_ = transform;
}

void SceneGraph::SetNodeBoundingVolumes(int nodeIndex,
    const glm::vec3& minBounds,
    const glm::vec3& maxBounds,
    const glm::vec3& sphereCenter,
    float sphereRadius) {
    if (nodeIndex < 0 || nodeIndex >= static_cast<int>(nodes_.size())) {
        Logger::GetLogger()->error("SetNodeBoundingVolumes: Invalid node index {}.", nodeIndex);
        throw std::out_of_range("Invalid node index in SetNodeBoundingVolumes");
    }
    nodes_[nodeIndex].boundingBoxMin_ = minBounds;
    nodes_[nodeIndex].boundingBoxMax_ = maxBounds;
    nodes_[nodeIndex].boundingSphereCenter_ = sphereCenter;
    nodes_[nodeIndex].boundingSphereRadius_ = sphereRadius;
}

void SceneGraph::AddMeshReference(int nodeIndex, int meshIndex, int materialIndex) {
    if (nodeIndex < 0 || nodeIndex >= static_cast<int>(nodes_.size())) {
        Logger::GetLogger()->error("AddMeshReference: Invalid node index {}.", nodeIndex);
        throw std::out_of_range("Invalid node index in AddMeshReference");
    }
    nodes_[nodeIndex].meshIndices_.push_back(meshIndex);
    nodes_[nodeIndex].materialIndices_.push_back(materialIndex);
}

void SceneGraph::RecalculateGlobalTransforms() {
    for (int i = 0; i < static_cast<int>(nodes_.size()); ++i) {
        if (nodes_[i].parentIndex_ == -1) {
            computeGlobalTransformRecursive(i, glm::mat4(1.0f));
        }
    }
}

void SceneGraph::computeGlobalTransformRecursive(int nodeIndex, const glm::mat4& parentGlobal) {
    auto& node = nodes_[nodeIndex];
    node.globalTransform_ = parentGlobal * node.localTransform_;
    for (int child : node.children_) {
        computeGlobalTransformRecursive(child, node.globalTransform_);
    }
}

void SceneGraph::TraverseGraph(const std::function<void(int nodeIndex)>& visitor) const {
    for (int i = 0; i < static_cast<int>(nodes_.size()); ++i) {
        visitor(i);
    }
}

void SceneGraph::TraverseGraphDFS(const std::function<bool(int nodeIndex)>& preVisitor) const {
    for (int i = 0; i < static_cast<int>(nodes_.size()); ++i) {
        if (nodes_[i].parentIndex_ == -1) {
            dfsTraversal(i, preVisitor);
        }
    }
}

void SceneGraph::dfsTraversal(int nodeIndex, const std::function<bool(int)>& preVisitor) const {
    if (!preVisitor(nodeIndex))
        return;
    for (int child : nodes_[nodeIndex].children_) {
        dfsTraversal(child, preVisitor);
    }
}