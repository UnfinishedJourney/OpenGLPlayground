#version 460 core

#include "Common/Material.shader"
#include "Common/Common.shader"

layout(location = 0) out vec4 color;

layout(binding = 0) uniform samplerCube u_Texture;

in vec3 wPos;
in vec3 wNormal;
in vec2 uv;

void main()
{

	vec3 uLightDirection = vec3(1.0, -1.0, 1.0);
	uLightDirection = normalize(uLightDirection);
	vec3 uLightColor = vec3(1.0);
	float PI = 3.14;
    // --- STEP 1: Base Material Properties ---
    vec3 Ka = uMaterial.Mtl0.xyz; // Ambient color fallback
    vec3 Kd = uMaterial.Mtl1.xyz; // Diffuse color fallback
    float alpha = uMaterial.Mtl1.w; // Opacity
    vec3 Ks = uMaterial.Mtl2.xyz; // Specular color fallback
    float Ns = uMaterial.Mtl2.w;  // Shininess
    vec3 Ke = uMaterial.Mtl3.xyz; // Emissive color fallback

    // --- STEP 2: Sample Textures (if available) ---
    // Albedo texture (bit 0)
    vec3 albedo = Kd;
    if (HasTexture(0))
    {
        albedo = texture(uTexAlbedo, uv).rgb;
    }

    // Normal mapping (bit 1): use texture if available, else use wNormal
    vec3 N = normalize(wNormal);
    //if (HasTexture(1))
    //{
    //    vec3 normalTex = texture(uTexNormal, uv).rgb;
    //    // Remap from [0,1] to [-1,1] and transform using the TBN matrix.
    //    N = normalize(TBN * (normalTex * 2.0 - 1.0));
    //}

    // Metalness and Roughness (bit 2): assume red channel is roughness, green channel is metallic.
    float metallic = 0.0;
    float roughness = 1.0;
    if (HasTexture(2))
    {
        vec2 mr = texture(uTexMetalRoughness, uv).rg;
        roughness = mr.r;
        metallic = mr.g;
    }

    // Ambient Occlusion (bit 3)
    float ao = 1.0;
    if (HasTexture(3))
    {
        ao = texture(uTexAO, uv).r;
    }

    // Emissive (bit 4)
    vec3 emissive = Ke;
    if (HasTexture(4))
    {
        emissive = texture(uTexEmissive, uv).rgb;
    }

    // --- STEP 3: Compute View Vector ---
    vec3 V = normalize(u_CameraPos.xyz - wPos);

    // --- STEP 4: Direct Lighting (using a single directional light) ---
    vec3 L = normalize(-uLightDirection); // Light comes from uLightDirection
    vec3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float VdotH = max(dot(V, H), 0.0);

    // Fresnel (Schlick’s approximation)
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 F = F0 + (1.0 - F0) * pow(1.0 - VdotH, 5.0);

    // GGX normal distribution function (NDF)
    float alphaSquared = roughness * roughness * roughness * roughness;
    float denom = (NdotH * NdotH) * (alphaSquared - 1.0) + 1.0;
    float D = alphaSquared / (PI * denom * denom);

    // Geometry function (Schlick-GGX)
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    float G_V = NdotV / (NdotV * (1.0 - k) + k);
    float G_L = NdotL / (NdotL * (1.0 - k) + k);
    float G = G_V * G_L;

    // Specular and diffuse components for direct lighting:
    vec3 specularDirect = (D * G * F) / (4.0 * NdotV * NdotL + 0.001);
    vec3 diffuseDirect = (1.0 - F) * (1.0 - metallic) * albedo / PI;

    vec3 directLighting = (diffuseDirect + specularDirect) * NdotL * uLightColor;

    // --- STEP 5: Image-Based Lighting (IBL) Diffuse ---
    // Here we use only a diffuse irradiance cubemap (u_EnvironmentMapDiffuse)
    vec3 irradiance = texture(u_EnvironmentMapDiffuse, N).rgb;
    vec3 ambientDiffuse = (1.0 - metallic) * albedo * irradiance;

    // --- STEP 6: Combine Lighting ---
    vec3 finalColor = directLighting + ambientDiffuse;
    finalColor = finalColor * ao + emissive;

    // --- STEP 7: Gamma Correction ---
    //finalColor = pow(finalColor, vec3(1.0 / 2.2));

    color = vec4(finalColor, alpha);
}