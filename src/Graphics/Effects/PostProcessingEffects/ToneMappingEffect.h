#pragma once
#include "Graphics/Effects/PostProcessingEffects/PostProcessingEffect.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <variant>
#include <glm/glm.hpp>

// The CPU–assisted tone mapping effect. It reads back the HDR image to compute the
// log–average luminance, then uses a GPU shader to perform the tone mapping operator.
class ToneMappingEffect : public PostProcessingEffect {
public:
    // Constructor: Pass in the fullscreen quad mesh and initial dimensions.
    ToneMappingEffect(std::shared_ptr<Graphics::MeshBuffer> quad, int width = 800, int height = 600);
    virtual ~ToneMappingEffect();

    // Apply the tone mapping effect.
    //   inputTexture: the HDR texture rendered in a high-precision framebuffer.
    //   outputFramebuffer: the target framebuffer (often the default framebuffer) for LDR output.
    virtual void Apply(GLuint inputTexture, GLuint outputFramebuffer) override;

    // Called when the window (and hence our render target) is resized.
    virtual void OnWindowResize(int width, int height) override;

    // Set tone mapping parameters via a parameter map.
    virtual void SetParameters(const std::unordered_map<std::string, EffectParameter>& params) override;

private:
    // Tone mapping parameters.
    float m_Exposure; // "a" value that acts like the exposure level.
    float m_LWhite;   // The white point that helps compress very bright pixels.

    // Computes the log–average luminance of the HDR image on the CPU.
    float ComputeLogAverageLuminance(GLuint hdrTexture);
};
