#version 460 core

// -----------------------------------------------------------------------------
// Uniforms and UBOs (make sure these are defined in your included files)
// -----------------------------------------------------------------------------
#include "Common/Common.shader"   // Should define u_CameraPos, etc.
#include "Common/Lights.shader"   // Should define lightsData[] and numLights

// -----------------------------------------------------------------------------
// Inputs / Outputs
// -----------------------------------------------------------------------------
in vec2  tc;          // Texture coordinates
in vec3  fragPos;     // World-space position
in vec3  normal;      // World-space normal (from geometry or partially from a normal map)
in mat3  TBN;         // Tangent-Bitangent-Normal matrix

out vec4 out_FragColor;

// -----------------------------------------------------------------------------
// Texture Samplers
// -----------------------------------------------------------------------------
layout(binding = 0) uniform samplerCube u_EnvironmentMap;
layout(binding = 1) uniform sampler2D texAlbedo;
layout(binding = 2) uniform sampler2D texNormal;
layout(binding = 3) uniform sampler2D texMetalRoughness; // .g = metallic, .r = roughness (here: .g = metallic, .r = roughness)
layout(binding = 4) uniform sampler2D texAO;
layout(binding = 5) uniform sampler2D texEmissive;

// BRDF LUT for specular IBL:
layout(binding = 8) uniform sampler2D texBRDF_LUT;

// Environment maps for IBL:
layout(binding = 9) uniform samplerCube u_EnvironmentMapDiffuse;   // Diffuse irradiance map
layout(binding = 10) uniform samplerCube u_EnvironmentMapSpecular;  // Prefiltered (specular) cubemap

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------
const float PI = 3.14159265359;

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
void main()
{
    // ----- STEP 1: Texture Sampling -----
    vec3 albedo     = texture(texAlbedo, tc).rgb;
    vec3 normalMap  = texture(texNormal, tc).rgb;
    vec2 metalRough = texture(texMetalRoughness, tc).bg; // .b = metallic, .g = roughness
    float metallic  = metalRough.y;
    float roughness = metalRough.x;
    float ao        = texture(texAO, tc).r;
    vec3 emissive   = texture(texEmissive, tc).rgb;

    // ----- STEP 2: Normal Mapping -----
    // Convert normal from [0,1] to [-1,1] then into world space via TBN.
    vec3 N = normalize(TBN * (normalMap * 2.0 - 1.0));

    // ----- STEP 3: View Vector -----
    vec3 V = normalize(u_CameraPos.xyz - fragPos);

    vec3 color = vec3(0.0);

    // ----- STEP 5: Ambient (IBL) Diffuse Lighting -----
    vec3 irradiance = texture(u_EnvironmentMapDiffuse, N).rgb;
    vec3 ambientDiffuse = (1.0 - metallic) * albedo * irradiance;
    color += ambientDiffuse;

    // ----- STEP 6: Ambient (IBL) Specular Lighting -----
    // Compute reflection vector.
    vec3 R = reflect(-V, N);
    //vec3 R = reflect(V, N);
    // Fresnel for environment reflection.
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    float VdotN = max(dot(V, N), 0.0);
    vec3 F_env = F0 + (1.0 - F0) * pow(1.0 - VdotN, 5.0);

    // Sample prefiltered (specular) environment.
    // We choose a maximum mip level count that should match the mip chain in the prefiltered cubemap.
    float maxMipLevels = 5.0;
    //vec3 prefilteredSpec = textureLod(u_EnvironmentMapSpecular, R, roughness * maxMipLevels).rgb;

    vec3 prefilteredSpec = texture(u_EnvironmentMap, R).rgb;


    // --- BRDF LUT Integration ---
    // Sample the BRDF LUT using the dot product N·V and the roughness.
    vec2 envBRDF = texture(texBRDF_LUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    // Combine the specular environment with the BRDF LUT:
    vec3 specularIBL = prefilteredSpec * (F_env * envBRDF.x + envBRDF.y);

    color += specularIBL;

    // ----- STEP 7: Apply Ambient Occlusion and Emissive -----
    color = color * ao + emissive;

    // ----- STEP 8: Output Final Color -----
    out_FragColor = vec4(color, 1.0);
}