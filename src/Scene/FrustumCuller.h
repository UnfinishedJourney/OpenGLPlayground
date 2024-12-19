#include <glm/glm.hpp>
#include <memory>

class FrustumCuller {
public:
    // For a more advanced approach, you'd extract planes from the camera's projection/view matrix.
    // For simplicity, let's define a helper function:
    bool IsSphereVisible(const glm::vec3& center, float radius, const std::shared_ptr<Camera>& camera) const {
        // Basic approach: check if (center) is within some distance from the camera
        // or do a real frustum-plane intersection test. We'll do a simple distance check:
        float distance = glm::distance(camera->GetPosition(), center);
        // If the distance - radius is beyond some huge range, we cull. E.g. 1000
        // (In real code, you'd extract planes from the camera frustum)
        float farPlane = camera->GetFarPlane();
        return (distance - radius) < farPlane;
    }
};