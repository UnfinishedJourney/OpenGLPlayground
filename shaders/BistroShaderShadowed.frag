#version 460 core

// We include your common material / lighting includes:
#include "Common/Common.shader"
#include "Common/Material.shader"
#include "Common/LightsFunctions.shader"

// We still may have a "samplerCube u_Texture" if you want, or rename it.
// But typically your environment maps are in separate binding slots.
layout(binding = 8) uniform samplerCube u_EnvironmentMap; // or you can rename

// For the shadow map
layout(binding = 10) uniform sampler2DShadow u_ShadowMap;

// PCF routine for directional shadow
float PCF(sampler2DShadow shadowMap, vec4 shadowCoord)
{
    // do the perspective divide
    vec3 sc = shadowCoord.xyz / shadowCoord.w;

    // if out of [0..1], no shadow (treat as lit)
    if(any(lessThan(sc, vec3(0.0))) || any(greaterThan(sc, vec3(1.0))))
        return 1.0;

    // 3x3 PCF
    float sum = 0.0;
    float texSize = 2048.0; // match your shadow map size
    float offset = 1.0 / texSize;

    for(int yy=-1; yy<=1; yy++)
    {
        for(int xx=-1; xx<=1; xx++)
        {
            vec4 offCoord = shadowCoord;
            offCoord.xy += vec2(xx, yy) * offset;
            sum += textureProj(shadowMap, offCoord);
        }
    }
    return sum / 9.0;
}

// We output final color
layout(location = 0) out vec4 color;

// Inputs from your vertex shader
in vec3 wPos;
in vec3 wNormal;
in vec2 uv;
// We'll also have the shadow coords from the vertex shader
in vec4 PosLightMap;
in mat3  TBN;          // TBN for normal mapping

// Start of main
void main()
{
    LightData ld = lightsData[0];
    vec3 uLightDirection = normalize(ld.position.xyz);
	vec3 uLightColor = ld.color.xyz;
	float PI = 3.14;
    // --- STEP 1: Base Material Properties ---
    vec3 Ka = uMaterial.Mtl0.xyz; // Ambient color fallback

    vec3 Kd = uMaterial.Mtl1.xyz; // Diffuse color fallback
    float alphaTest = uMaterial.Mtl1.w; // Opacity
    vec3 Ks = uMaterial.Mtl2.xyz; // Specular color fallback
    float Ns = uMaterial.Mtl2.w;  // Shininess
    vec3 Ke = uMaterial.Mtl3.xyz; // Emissive color fallback


    // 2. Base color & alpha
    vec3 albedo  = Kd;
    float alpha  = 1.0;  // default
    if (HasTexture(0))   // bit 0 means there's an Albedo texture
    {
        vec4 texColor = texture(uTexAlbedo, uv);
        //color = texColor;
        //return;
        albedo = Kd*texColor.rgb;
        alpha  = texColor.a;
    }

    if (alpha + 0.2 < alphaTest)
        discard;

    // Normal mapping (bit 1): use texture if available, else use wNormal
    vec3 N = normalize(wNormal);
    if (HasTexture(1))
    {
        vec3 normalTex = texture(uTexNormal, uv).rgb;
        //Remap from [0,1] to [-1,1] and transform using the TBN matrix.
        N = normalize(TBN * (normalTex * 2.0 - 1.0));
        color = vec4(1.0, 1.0, 0.0, 1.0);
        return;
    }

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
        vec4 texColor = texture(uTexEmissive, uv);
        emissive = Ke*texColor.rgb;
    }

    if (HasTexture(1))
    {
        vec4 texColor = texture(uTexHeight, uv);
        color = vec4(1.0, 0.0, 1.0, 1.0);
        return;
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
    vec3 diffuseDirect = (1.0 - F) * (1.0 - metallic) * albedo;


    float shadowFactor = PCF(u_ShadowMap, PosLightMap);

    vec3 directLighting = shadowFactor * (diffuseDirect + specularDirect) * NdotL * uLightColor;

    // --- STEP 5: Image-Based Lighting (IBL) Diffuse ---
    // Here we use only a diffuse irradiance cubemap (u_EnvironmentMapDiffuse)
    vec3 irradiance = texture(u_EnvironmentMapDiffuse, N).rgb;
    vec3 ambientDiffuse = (1.0 - metallic) * albedo * irradiance;

    //color = vec4(shadowFactor, 0.0, 0.0, 1.0);
    //return;
    // --- STEP 6: Combine Lighting ---
    vec3 finalColor = (directLighting + ambientDiffuse)*1.5;
    finalColor = finalColor * ao + emissive;

    // --- STEP 7: Gamma Correction ---
    finalColor = pow(finalColor, vec3(1.0 / 2.2));

    color = vec4(finalColor, 1.0);
}