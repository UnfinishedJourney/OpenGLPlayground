#include "FrustumCuller.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <cmath>

void FrustumCuller::ExtractFrustumPlanes(const glm::mat4& projViewMatrix)
{
    // Left
    m_Planes[0].a = projViewMatrix[0][3] + projViewMatrix[0][0];
    m_Planes[0].b = projViewMatrix[1][3] + projViewMatrix[1][0];
    m_Planes[0].c = projViewMatrix[2][3] + projViewMatrix[2][0];
    m_Planes[0].d = projViewMatrix[3][3] + projViewMatrix[3][0];

    // Right
    m_Planes[1].a = projViewMatrix[0][3] - projViewMatrix[0][0];
    m_Planes[1].b = projViewMatrix[1][3] - projViewMatrix[1][0];
    m_Planes[1].c = projViewMatrix[2][3] - projViewMatrix[2][0];
    m_Planes[1].d = projViewMatrix[3][3] - projViewMatrix[3][0];

    // Bottom
    m_Planes[2].a = projViewMatrix[0][3] + projViewMatrix[0][1];
    m_Planes[2].b = projViewMatrix[1][3] + projViewMatrix[1][1];
    m_Planes[2].c = projViewMatrix[2][3] + projViewMatrix[2][1];
    m_Planes[2].d = projViewMatrix[3][3] + projViewMatrix[3][1];

    // Top
    m_Planes[3].a = projViewMatrix[0][3] - projViewMatrix[0][1];
    m_Planes[3].b = projViewMatrix[1][3] - projViewMatrix[1][1];
    m_Planes[3].c = projViewMatrix[2][3] - projViewMatrix[2][1];
    m_Planes[3].d = projViewMatrix[3][3] - projViewMatrix[3][1];

    // Near
    m_Planes[4].a = projViewMatrix[0][3] + projViewMatrix[0][2];
    m_Planes[4].b = projViewMatrix[1][3] + projViewMatrix[1][2];
    m_Planes[4].c = projViewMatrix[2][3] + projViewMatrix[2][2];
    m_Planes[4].d = projViewMatrix[3][3] + projViewMatrix[3][2];

    // Far
    m_Planes[5].a = projViewMatrix[0][3] - projViewMatrix[0][2];
    m_Planes[5].b = projViewMatrix[1][3] - projViewMatrix[1][2];
    m_Planes[5].c = projViewMatrix[2][3] - projViewMatrix[2][2];
    m_Planes[5].d = projViewMatrix[3][3] - projViewMatrix[3][2];

    // Normalize them
    for (auto& plane : m_Planes) {
        NormalizePlane(plane);
    }
}

void FrustumCuller::NormalizePlane(Plane& plane)
{
    float length = std::sqrt(plane.a * plane.a + plane.b * plane.b + plane.c * plane.c);
    if (length != 0.0f) {
        plane.a /= length;
        plane.b /= length;
        plane.c /= length;
        plane.d /= length;
    }
}

bool FrustumCuller::IsSphereVisible(const glm::vec3& center, float radius) const
{
    for (auto& plane : m_Planes) {
        float dist = PlaneDistance(plane, center);
        if (dist < -radius) {
            // outside
            return false;
        }
    }
    return true;
}