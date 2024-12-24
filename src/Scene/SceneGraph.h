#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <functional>

/**
 * SceneGraphNode stores:
 *  - parent and children
 *  - a local transform (plus a cached global transform)
 *  - optional bounding volumes (for culling)
 *  - references to mesh(es), material index, or other data
 *
 * You might store multiple meshes per node (like in your Model::Node).
 */
struct SceneGraphNode
{
    int parent = -1;
    std::vector<int> children;

    std::string name;

    // Transforms
    glm::mat4 localTransform = glm::mat4(1.0f);
    glm::mat4 globalTransform = glm::mat4(1.0f);

    // For culling
    glm::vec3 boundingBoxMin = glm::vec3(0.0f);
    glm::vec3 boundingBoxMax = glm::vec3(0.0f);
    glm::vec3 boundingSphereCenter = glm::vec3(0.0f);
    float boundingSphereRadius = 0.0f;

    // If a node can hold multiple meshes
    std::vector<int> meshIndices;    // references into your m_LoadedMeshes array
    std::vector<int> materialIndices; // references to your m_LoadedMaterials
};

/**
 * The SceneGraph manages an array of SceneGraphNode.
 * For LOD/culling, you can store bounding volumes per node.
 */
class SceneGraph
{
public:
    // Add a node under 'parent'. If parent == -1, it's a root node.
    int AddNode(int parent, const std::string& name);

    // Set the local transform (like from your Model's node transform).
    void SetLocalTransform(int nodeIndex, const glm::mat4& transform);

    // You can store bounding volumes if you want to cull at the node level
    void SetNodeBoundingVolumes(int nodeIndex,
        const glm::vec3& minB,
        const glm::vec3& maxB,
        const glm::vec3& sphereCenter,
        float sphereRadius);

    // If your node references one or more meshes
    void AddMeshReference(int nodeIndex, int meshIndex, int materialIndex);

    // Recompute global transforms from the root down.
    // Usually called after loading or after modifying transforms.
    void RecalculateGlobalTransforms();

    // A “visitor” or DFS approach so you can do culling or LOD logic
    void TraverseGraph(std::function<void(int nodeIndex)> visitor);

    // For convenience
    const std::vector<SceneGraphNode>& GetNodes() const { return m_Nodes; }

private:
    // BFS or DFS utility
    void computeGlobalTransformRecursive(int nodeIndex, const glm::mat4& parentGlobal);

private:
    std::vector<SceneGraphNode> m_Nodes;
};