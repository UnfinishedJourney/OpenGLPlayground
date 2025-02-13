#pragma once

#include <vector>
#include <memory>
#include <optional>

#include "glm/glm.hpp"
#include "Lights.h"

namespace graphics {
    class UniformBuffer;
    class ShaderStorageBuffer;
};

//maybe should rewrite it

struct BoundingBox
{
    glm::vec3 min_{ 0.0f };
    glm::vec3 max_{ 0.0f };

    void combinePoint(const glm::vec3& p) {
        min_ = glm::min(min_, p);
        max_ = glm::max(max_, p);
    }
};

/**
 * @brief LightManager manages a list of Light objects.
 *        - Builds GPU data for them (LightData).
 *        - Computes directional light's orthographic frustum.
 *        - Computes point light's perspective frustum. (need to think about better calculation)
 *        - Optionally manages shadow resources (e.g. FBOs) for each light.
 */

struct alignas(16) LightData {
    glm::vec4 position_; // w is padding or another parameter, for dir lights position is direction? don't have them yet
    glm::vec4 color_;    // w is intensity
};

class LightManager
{
public:
    LightManager();
    ~LightManager();

    // Possibly remove or update lights, etc.


    // This function updates/binds the GPU buffer (SSBO) 
    void UpdateLightsGPU();
    void BindLightsGPU() const;

    void SetBoundingBox(glm::vec3 min, glm::vec3 max) {
        bBox_.min_ = min;
        bBox_.max_ = max;
    }

    glm::mat4 ComputeLightView(size_t id) const;
    glm::mat4 ComputeLightProj(size_t id) const;

    //maybe should utilize light class
    // 
    // we get light id, index in vector
    std::optional<size_t> AddLight(const LightData& light);
    const std::vector<LightData>& GetLightsData() const { return lightsData_; }

private:

    BoundingBox bBox_;

    //std::vector<Light> m_Lights;

    // Store GPU buffer here :
    std::unique_ptr<graphics::ShaderStorageBuffer> lightsSSBO_;

    std::vector<LightData>  lightsData_;


    // For a directional light, we can compute an orthographic matrix that encloses the entire bounding box
    // This is one example:
    glm::mat4 ComputeDirectionalLightView(const LightData& light) const;
    glm::mat4 ComputeDirectionalLightProj(const LightData& light) const;

    // For a point light, we can compute an perspective matrix (need to think)
    // This is one example:
    glm::mat4 ComputePointLightView(const LightData& light) const;
    glm::mat4 ComputePointLightProj(const LightData& light) const;
};