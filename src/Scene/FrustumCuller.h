#pragma once
#include <glm/glm.hpp>
#include <array>

class Camera;

/**
 * Simple frustum culler that extracts 6 planes (left, right, bottom, top, near, far)
 * from the camera’s projection * view matrix. Then does sphere-plane tests.
 */
class FrustumCuller {
public:
    // Call once per frame to update planes
    void ExtractFrustumPlanes(const glm::mat4& projViewMatrix);

    // Return true if a bounding sphere is inside (or intersects) the frustum
    bool IsSphereVisible(const glm::vec3& center, float radius) const;

private:
    // Each plane: ax + by + cz + d = 0
    struct Plane {
        float a, b, c, d;
    };

    std::array<Plane, 6> m_Planes;

    void NormalizePlane(Plane& plane);
    float PlaneDistance(const Plane& plane, const glm::vec3& point) const {
        return plane.a * point.x + plane.b * point.y + plane.c * point.z + plane.d;
    }
};