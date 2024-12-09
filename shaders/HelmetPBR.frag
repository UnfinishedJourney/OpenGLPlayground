#version 460 core

#include "Common/Common.shader"
#include "Common/Lights.shader"

// **2. Define Inputs and Outputs**

in vec2 tc;           // Texture coordinates
in vec3 fragPos;      // Fragment position in world space
in vec3 normal;       // Normal in world space
in mat3 TBN;          // TBN matrix

out vec4 out_FragColor; // Output fragment color

// **3. Define Samplers**

layout(binding = 0) uniform sampler2D texBRDF_LUT;
layout(binding = 1) uniform sampler2D texAlbedo;
layout(binding = 2) uniform sampler2D texNormal;
layout(binding = 3) uniform sampler2D texMetalRoughness;
layout(binding = 4) uniform sampler2D texAO;
layout(binding = 5) uniform sampler2D texEmissive;

// **4. Define Constants**

const float PI = 3.14159265359;

// **5. Main Function**

void main()
{
    // **Step 1: Sampling Textures**
    
    // Albedo color
    vec3 albedo = texture(texAlbedo, tc).rgb;
    // Normal from normal map
    vec3 normalMap = texture(texNormal, tc).rgb;
    // Metallic and roughness from the respective texture
    vec2 metalRough = texture(texMetalRoughness, tc).gb; // g = Roughness, b = Metallic
    float metallic = metalRough.y;
    float roughness = metalRough.x;
    // Ambient Occlusion coefficient
    float ao = texture(texAO, tc).r;
    // Emissive color
    vec3 emissive = texture(texEmissive, tc).rgb;
    // BRDF LUT (if used for more advanced calculations)
    vec3 lut = texture(texBRDF_LUT, tc).rgb; // Ensure you utilize this if needed
    
    // **Step 2: Transforming Normal from Tangential to World Space**
    
    // Convert normal from [0,1] range to [-1,1]
    vec3 N = normalMap * 2.0 - 1.0;
    N = normalize(TBN * N); // Transform normal to world space
    
    // **Step 3: Calculating View Vector**
    
    vec3 V = normalize(u_CameraPos.xyz - fragPos); // View vector (from fragment to camera)
    
    // **Step 4: Initialize Lighting Accumulator**
    
    vec3 color = vec3(0.0);
    
    // **Step 5: Iterate Over Lights and Accumulate Lighting**
    
    for (uint i = 0; i < numLights; ++i)
    {
        // Extract light data
        vec3 L = normalize(lightsData[i].position.xyz - fragPos); // Light vector
        vec3 lightColor = lightsData[i].color.rgb * lightsData[i].color.a; // Color multiplied by intensity
        
        vec3 H = normalize(V + L); // Half-vector
    
        // **Dot Products**
    
        float NdotL = max(dot(N, L), 0.0);
        float NdotV = max(dot(N, V), 0.0);
        float NdotH = max(dot(N, H), 0.0);
        float LdotH = max(dot(L, H), 0.0);
        float VdotH = max(dot(V, H), 0.0);
    
        // **BRDF Components**
    
        // Fresnel reflection
        vec3 F0 = mix(vec3(0.04), albedo, metallic); // Base reflection
        vec3 F = F0 + (1.0 - F0) * pow(1.0 - VdotH, 5.0); // Schlick's approximation
    
        // Normal Distribution Function (NDF) — using GGX
        float alpha = roughness * roughness;
        float alpha2 = alpha * alpha;
        float denom = (NdotH * NdotH) * (alpha2 - 1.0) + 1.0;
        float D = alpha2 / (PI * denom * denom);
    
        // Geometry function (G) — using Schlick-GGX
        float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
        float G_NdotV = NdotV / (NdotV * (1.0 - k) + k);
        float G_NdotL = NdotL / (NdotL * (1.0 - k) + k);
        float G = G_NdotV * G_NdotL;
    
        // Specular component
        vec3 numerator = D * G * F;
        float denominator = 4.0 * NdotV * NdotL + 0.001; // Adding a small number to avoid division by zero
        vec3 specular = numerator / denominator;
    
        // Diffuse component
        vec3 kd = vec3(1.0) - F;        // Diffuse reflection coefficient
        kd *= 1.0 - metallic;            // Metals do not have a diffuse component
    
        vec3 diffuse = kd * albedo / PI; // Lambertian diffuse model
    
        // Accumulate lighting
        color += (diffuse + specular) * NdotL * lightColor;
    }
    
    // **Step 6: Applying Ambient Occlusion and Emissive Color**
    
    //color *= ao;        // Apply Ambient Occlusion
    color += emissive;  // Add emissive color
    
    // **Step 7: Gamma Correction**
    
    color = pow(color, vec3(1.0/2.2)); // Convert from linear space to sRGB
    
    // **Step 8: Outputting the Result**
    
    out_FragColor = vec4(color, 1.0); // Set alpha channel to 1.0
}
