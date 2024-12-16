#include "SceneNode.h"
#include <algorithm>

SceneNode::SceneNode()
    : m_LocalTransform(1.0f)
{}

void SceneNode::AddChild(const std::shared_ptr<SceneNode>& child)
{
    m_Children.push_back(child);
}

void SceneNode::RemoveChild(const std::shared_ptr<SceneNode>& child)
{
    auto it = std::find(m_Children.begin(), m_Children.end(), child);
    if (it != m_Children.end()) {
        m_Children.erase(it);
    }
}

glm::mat4 SceneNode::CalculateWorldTransform(const glm::mat4& parentTransform) const
{
    return parentTransform * m_LocalTransform;
}