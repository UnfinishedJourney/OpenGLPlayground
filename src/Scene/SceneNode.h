#pragma once
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Renderer/RenderObject.h"

/**
 * A node in the scene graph. Each node has:
 * - A local transform (glm::mat4)
 * - Zero or one RenderObject pointer
 * - A list of children
 */
class SceneNode {
public:
    SceneNode();
    ~SceneNode() = default;

    // Hierarchy
    void AddChild(const std::shared_ptr<SceneNode>& child);
    void RemoveChild(const std::shared_ptr<SceneNode>& child);
    const std::vector<std::shared_ptr<SceneNode>>& GetChildren() const { return m_Children; }

    // Local transform
    void SetLocalTransform(const glm::mat4& transform) { m_LocalTransform = transform; }
    const glm::mat4& GetLocalTransform() const { return m_LocalTransform; }

    // World transform (computed by parent * local)
    glm::mat4 CalculateWorldTransform(const glm::mat4& parentTransform = glm::mat4(1.0f)) const;

    // RenderObject (optional)
    void SetRenderObject(const std::shared_ptr<RenderObject>& ro) { m_RenderObject = ro; }
    std::shared_ptr<RenderObject> GetRenderObject() const { return m_RenderObject; }

private:
    glm::mat4 m_LocalTransform;
    std::shared_ptr<RenderObject> m_RenderObject;
    std::vector<std::shared_ptr<SceneNode>> m_Children;
};