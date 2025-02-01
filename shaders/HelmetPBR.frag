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
layout(binding = 1) uniform sampler2D texAlbedo;
layout(binding = 2) uniform sampler2D texNormal;
layout(binding = 3) uniform sampler2D texMetalRoughness; // .g = metallic, .r or .x = roughness (here we use .g for metal, .r for rough)
layout(binding = 4) uniform sampler2D texAO;
layout(binding = 5) uniform sampler2D texEmissive;
// (Optional: a BRDF LUT sampler if you add a specular IBL later)
// layout(binding = 8) uniform sampler2D texBRDF_LUT;

// The irradiance cubemap for diffuse IBL (bound at unit 0)
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
    // ----- STEP 1: Texture Sampling -----
    vec3 albedo     = texture(texAlbedo, tc).rgb;
    vec3 normalMap  = texture(texNormal, tc).rgb;
    vec2 metalRough = texture(texMetalRoughness, tc).bg;  // Note: .b = metallic, .g = roughness 
    float metallic  = metalRough.y;
    float roughness = metalRough.x;  
    float ao        = texture(texAO, tc).r;
    vec3 emissive   = texture(texEmissive, tc).rgb;

    // ----- STEP 2: Normal Mapping -----
    // Transform normal from [0,1] to [-1,1] then into world space using the TBN matrix.
    vec3 N = normalize(TBN * (normalMap * 2.0 - 1.0));

    // ----- STEP 3: View Vector -----
    vec3 V = normalize(u_CameraPos.xyz - fragPos);

    // ----- STEP 4: Direct Lighting from Scene Lights -----
    vec3 color = vec3(0.0);
    for (uint i = 0; i < numLights; ++i)
    {
        // Calculate light direction and other vectors.
        vec3 L = normalize(lightsData[i].position.xyz - fragPos);
        vec3 lightColor = lightsData[i].color.rgb * lightsData[i].color.a;
        vec3 H = normalize(V + L);

        // Dot products.
        float NdotL = max(dot(N, L), 0.0);
        float NdotV = max(dot(N, V), 0.0);
        float NdotH = max(dot(N, H), 0.0);
        float VdotH = max(dot(V, H), 0.0);

        // Fresnel using Schlick’s approximation.
        vec3 F0 = mix(vec3(0.04), albedo, metallic);
        vec3 F = F0 + (1.0 - F0) * pow(1.0 - VdotH, 5.0);

        // Normal Distribution Function (GGX).
        float alpha = roughness * roughness;
        float alpha2 = alpha * alpha;
        float denom = (NdotH * NdotH) * (alpha2 - 1.0) + 1.0;
        float D = alpha2 / (PI * denom * denom);

        // Geometry function (Schlick-GGX).
        float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
        float G1 = NdotV / (NdotV * (1.0 - k) + k);
        float G2 = NdotL / (NdotL * (1.0 - k) + k);
        float G = G1 * G2;

        // Specular term.
        vec3 numerator = D * G * F;
        float denominator = 4.0 * NdotV * NdotL + 0.0001;
        vec3 specular = numerator / denominator;

        // Diffuse term (Lambertian).
        vec3 kd = (1.0 - F) * (1.0 - metallic);
        vec3 diffuse = kd * albedo / PI;

        // Accumulate the lighting contribution.
        color += (diffuse + specular) * NdotL * lightColor;
    }

    // ----- STEP 5: Ambient (IBL) Diffuse Lighting -----
    // Since we do not have a specular cubemap, we only sample the irradiance map
    // for the diffuse ambient contribution.
    vec3 irradiance = texture(u_EnvironmentMap, N).rgb;
    vec3 ambientDiffuse = (1.0 - metallic) * albedo * irradiance;

    color += ambientDiffuse;

    // ----- STEP 6: Apply Ambient Occlusion and Emissive -----
    color = color * ao + emissive;

    // ----- STEP 7: Output Final Color -----
    out_FragColor = vec4(color, 1.0);
}