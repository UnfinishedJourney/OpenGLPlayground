#version 460 core

#include "Common/Common.shader"
#include "Common/LightsFunctions.shader"

layout(location = 0) out vec4 color;

in vec4 PosLightMap;  // Shadow coords (after bias)
// Inputs from the vertex shader
in vec3 FragPos;
in vec3 Normal;

////////////////////////////////////////////////////////////////////////////////
// 1) Use a shadow sampler!
////////////////////////////////////////////////////////////////////////////////
layout(binding = 10) uniform sampler2DShadow u_ShadowMap;

void main()
{
    color = vec4(0.3, 0.5, 0.8, 1.0);

    vec3 viewDir = normalize(u_CameraPos.xyz - FragPos);
    vec3 norm    = normalize(Normal);

    color = vec4(norm, 1.0);
    //return;
    vec3 lighting = CalculateBlinnPhongLighting(norm, viewDir, FragPos);

    color = vec4(lighting, 1.0);

        ////////////////////////////////////////////////////////////////////////////
    // 3) Sample the shadow map with textureProj
    //    Because it's a sampler2DShadow, textureProj(...) returns float
    //    in [0..1], representing how much is lit vs. shadowed.
    ////////////////////////////////////////////////////////////////////////////
    float shadowFactor = textureProj(u_ShadowMap, PosLightMap);
    // shadowFactor typically is 1.0 if fully lit, 0.0 if in shadow,
    // or partial if the hardware does PCF.

    ////////////////////////////////////////////////////////////////////////////
    // 4) Combine lighting with the shadow factor
    ////////////////////////////////////////////////////////////////////////////
    // For example, multiply the diffuse portion by (shadowFactor)
    vec3 finalColor = shadowFactor * lighting;


    color = vec4(finalColor, 1.0);


    //color = vec4(shadowFactor, 0.0, 0.0, 1.0);


}