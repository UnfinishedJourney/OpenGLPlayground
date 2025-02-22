#version 460 core

// We include your common material / lighting includes:
#include "Common/Common.shader"
#include "Common/Material.shader"
#include "Common/LightsFunctions.shader"
#include "Common/Parallax.shader"
#include "Common/PCF.shader"
#include "Common/PBR.shader"

// For the shadow map
layout(binding = 10) uniform sampler2DShadow u_ShadowMap;

// We output final color
layout(location = 0) out vec4 out_FragColor;

// Inputs from your vertex shader
in vec3 wPos;
in vec3 wNormal;
in vec2 uv;
// Shadow coordinates
in vec4 PosLightMap;
in mat3  TBN;

const float PI = 3.14159265;

void main()
{
    // --- Retrieve Light Data ---
    LightData ld = lightsData[0];
    vec3 lightDirection = normalize(ld.position.xyz);
    vec3 lightColor = ld.color.xyz;

    // --- Material Fallbacks and Textures ---
    vec3 albedo = uMaterial.Mtl1.xyz;
    float alphaTest = uMaterial.Mtl1.w;
    vec4 texDiffuse = vec4(1.0);
    vec2 uvParallax = uv;

    // --- Parallax Mapping ---
    if (HasTexture(6)) {
        vec3 Vworld = normalize(u_CameraPos.xyz - wPos);
        vec3 V_tangent = normalize(TBN * Vworld);
        uvParallax = calculateUVSimpleParallax(uTexHeight, uv, V_tangent);
    }

    // --- Diffuse Texture ---
    if (HasTexture(0)) {
        texDiffuse = texture(uTexDiffuse, uvParallax);
        texDiffuse.rgb = SRGBtoLINEAR(texDiffuse).rgb;
        albedo *= texDiffuse.rgb;
        if (texDiffuse.a < 0.1) discard;
    }

    // --- Ambient Occlusion ---
    float ao = 1.0;
    if (HasTexture(3)) ao = texture(uTexAO, uvParallax).r;

    // --- Emissive ---
    vec3 emissive = uMaterial.Mtl3.xyz;
    if (HasTexture(4)) {
        vec4 texEmissive = texture(uTexEmissive, uvParallax);
        texEmissive.rgb = SRGBtoLINEAR(texEmissive).rgb;
        emissive *= texEmissive.rgb;
    }

    // --- Normal Mapping ---
    vec3 N = normalize(wNormal);
    if (HasTexture(1)) {
        vec3 normalSample = texture(uTexNormal, uvParallax).rgb;
        N = normalize(TBN * (normalSample * 2.0 - 1.0));
    }

    // --- Specular and Roughness from Texture ---
    float metallic = 0.0;
    float roughness = 1.0;
    vec3 specularF0 = mix(vec3(0.04), albedo, metallic);
    
    if (HasTexture(2)) {
        vec4 specMap = texture(uTexMetalRoughness, uvParallax);
        specularF0 = specMap.rgb;  // RGB channels contain specular color
        roughness = specMap.a;     // Alpha channel contains roughness
        metallic = 0.0;           // Explicitly set metallic for specular workflow
    }

    // --- Lighting Calculations ---
    vec3 V = normalize(u_CameraPos.xyz - wPos);
    vec3 L = normalize(-lightDirection);
    vec3 H = normalize(V + L);

    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float VdotH = max(dot(V, H), 0.0);

    // --- Fresnel with Specular Map ---
    vec3 F = specularF0 + (1.0 - specularF0) * pow(1.0 - VdotH, 5.0);

    // --- GGX NDF ---
    float alpha = roughness * roughness;
    float alphaSquared = alpha * alpha;
    float denom = (NdotH * NdotH) * (alphaSquared - 1.0) + 1.0;
    float D = alphaSquared / (PI * denom * denom);

    // --- Geometry Function ---
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    float G_V = NdotV / (NdotV * (1.0 - k) + k);
    float G_L = NdotL / (NdotL * (1.0 - k) + k);
    float G = G_V * G_L;

    // --- Specular Term ---
    vec3 specular = (D * G * F) / (4.0 * NdotV * NdotL + 0.001);

    // --- Diffuse Term ---
    vec3 diffuse = (1.0 - F) * (1.0 - metallic) * albedo;

    // --- Shadow Factor ---
    float shadowFactor = PCF(u_ShadowMap, PosLightMap, 5);
    //out_FragColor = vec4(shadowFactor, 0.0, 0.0, 1.0);
    //return;
    // --- Lighting Composition ---
    vec3 directLighting = shadowFactor * (diffuse + specular) * NdotL * lightColor;
    vec3 irradiance = texture(uTexIrradianceMap, N).rgb;
    vec3 ambient = (1.0 - metallic) * albedo * irradiance;
    
    vec3 finalColor = (directLighting + ambient) * ao + emissive;
    out_FragColor = vec4(finalColor, 1.0);
}