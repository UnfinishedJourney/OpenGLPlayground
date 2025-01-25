#version 460 core

// -----------------------------------------------------------------------------
// Common Uniforms and UBO
// -----------------------------------------------------------------------------
#include "Common/Common.shader"
#include "Common/Lights.shader"

// (This block typically looks like:)
// layout(std140, binding = 0) uniform FrameData {
//     mat4 u_View;
//     mat4 u_Proj;
//     vec4 u_CameraPos;
// };
// struct LightData { ... };
// uniform Lights { LightData lightsData[MAX_LIGHTS]; uint numLights; };

// -----------------------------------------------------------------------------
// Inputs / Outputs
// -----------------------------------------------------------------------------
in vec2  tc;           // Texture coords
in vec3  fragPos;      // World-space position
in vec3  normal;       // World-space normal (from geometry or partial normal map)
in mat3  TBN;          // TBN for normal mapping

out vec4 out_FragColor;

// -----------------------------------------------------------------------------
// Material / Texture Samplers
// -----------------------------------------------------------------------------
layout(binding = 1) uniform sampler2D texAlbedo;
layout(binding = 2) uniform sampler2D texNormal;
layout(binding = 3) uniform sampler2D texMetalRoughness;
layout(binding = 4) uniform sampler2D texAO;
layout(binding = 5) uniform sampler2D texEmissive;
layout(binding = 8) uniform sampler2D texBRDF_LUT;

// **NEW**: environment cubemap (the skybox used for IBL)
layout(binding = 0) uniform samplerCube u_EnvironmentMap;

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------
const float PI = 3.14159265359;

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
void main()
{
    // STEP 1: Sample Textures
    vec3 albedo     = texture(texAlbedo, tc).rgb;

    vec3 normalMap  = texture(texNormal, tc).rgb;
    vec2 metalRough = texture(texMetalRoughness, tc).gb; // .g=rough, .b=metal
    float metallic  = metalRough.y;
    float roughness = metalRough.x;
    float ao        = texture(texAO, tc).r; 
    vec3 emissive   = texture(texEmissive, tc).rgb;
    // BRDF LUT (optional advanced usage)
    vec3 brdfSample = texture(texBRDF_LUT, tc).rgb; // might be used later

    out_FragColor = vec4(brdfSample, 1.0);
    return;
    // Convert normal map from [0..1] to [-1..1], then transform to world space
    vec3 N = normalMap * 2.0 - 1.0;
    N = normalize(TBN * N);

    // View vector
    vec3 V = normalize(u_CameraPos.xyz - fragPos);

    // Initialize color accumulator
    vec3 color = vec3(0.0);

    // -------------------------------------------------------------------------
    // STEP 2: Direct Lighting from Scene Lights
    // -------------------------------------------------------------------------
    for (uint i = 0; i < numLights; ++i)
    {
        vec3 L = normalize(lightsData[i].position.xyz - fragPos);
        vec3 lightColor = lightsData[i].color.rgb * lightsData[i].color.a; 
        vec3 H = normalize(V + L);

        // Dot products
        float NdotL = max(dot(N, L), 0.0);
        float NdotV = max(dot(N, V), 0.0);
        float NdotH = max(dot(N, H), 0.0);
        float VdotH = max(dot(V, H), 0.0);

        // Fresnel (Schlick)
        vec3 F0 = mix(vec3(0.04), albedo, metallic);
        vec3 F  = F0 + (1.0 - F0) * pow(1.0 - VdotH, 5.0);

        // Distribution (D) - GGX
        float alpha  = roughness * roughness;
        float alpha2 = alpha * alpha;
        float denom  = (NdotH * NdotH) * (alpha2 - 1.0) + 1.0;
        float D      = alpha2 / (PI * denom * denom);

        // Geometry (G) - Schlick‐GGX
        float k      = (roughness + 1.0)*(roughness + 1.0) / 8.0;
        float G1     = NdotV / (NdotV*(1.0 - k) + k);
        float G2     = NdotL / (NdotL*(1.0 - k) + k);
        float G      = G1 * G2;

        // Specular term
        vec3 numerator   = D * G * F;
        float denominator= 4.0 * NdotV * NdotL + 0.0001;
        vec3 specular    = numerator / denominator;

        // Diffuse term (Lambertian)
        vec3 kd = (1.0 - F) * (1.0 - metallic); // metals have minimal diffuse
        vec3 diffuse = kd * albedo / PI;

        // Accumulate direct lighting
        color += (diffuse + specular) * NdotL * lightColor;
    }

    // -------------------------------------------------------------------------
    // STEP 3: Environment Lighting from the Skybox
    // -------------------------------------------------------------------------
    // For a correct PBR approach, you'd use a *prefiltered* env map for specular
    // and an *irradiance* map for diffuse. We'll do a simplified approach:

    // Reflection vector
    vec3 R = reflect(-V, N);

    // Fresnel for environment reflection
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    float VdotN = max(dot(V, N), 0.0);
    vec3 F_env = F0 + (1.0 - F0) * pow(1.0 - VdotN, 5.0);

    // Sample the cubemap for reflection (roughness not accounted for here!)
    // For physically‐based, you'd do textureLod(u_EnvironmentMap, R, roughness*MIPLEVELS).
    vec3 envSpec = texture(u_EnvironmentMap, R).rgb;
    // Weighted by Fresnel
    vec3 specularIBL = F_env * envSpec;

    // Quick hack: sample the same cubemap for diffuse by just using N
    // (Real PBR would use a convolved irradiance cubemap.)
    vec3 envDiffuse = (1.0 - F_env) * (1.0 - metallic) * texture(u_EnvironmentMap, N).rgb;

    // Add environment contributions
    color += envSpec * 0.5;     // or specularIBL if you prefer
    color += envDiffuse * 0.3;  // scale down for test, or skip

    // -------------------------------------------------------------------------
    // STEP 4: Apply AO and Emissive
    // -------------------------------------------------------------------------
    color *= ao;
    color += emissive;

    out_FragColor = vec4(color, 1.0);
}