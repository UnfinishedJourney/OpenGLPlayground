#version 460 core

#include "Common/Common.shader"
#include "Common/LightsFunctions.shader"

layout(location = 0) out vec4 color;

in vec4 PosLightMap;  // Shadow coords (after bias)
// Inputs from the vertex shader
in vec3 FragPos;
in vec3 Normal;

layout(binding = 10) uniform sampler2DShadow u_ShadowMap;

float PCF(sampler2DShadow shadowMap, vec4 shadowCoord)
{
    float depth = 0.01;
    // do the perspective divide manually:
    vec3 coord = shadowCoord.xyz / shadowCoord.w;

    // If outside 0..1 range -> not in shadow map => shadow=1
    if(any(lessThan(coord, vec3(0.0))) || any(greaterThan(coord, vec3(1.0))))
        return 1.0;

    float shadowSum = 0.0;
    const float texSize = 2048.0; // match your shadow map resolution
    float offset = 1.0 / texSize; 
    // 3x3 sample
    for(int y=-1; y<=1; y++)
    {
        for(int x=-1; x<=1; x++)
        {
            vec4 offCoord = shadowCoord;
            offCoord.xy += vec2(x, y) * offset;
            shadowSum += textureProj(shadowMap, offCoord);
        }
    }
    return shadowSum / 9.0;
}

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

    
    vec3 projCoord = PosLightMap.xyz / PosLightMap.w;  // manual perspective divide
    // This is in the [-1..1] clip space if you haven't applied the bias yet
    // or in [0..1] if you have multiplied by the bias matrix. Adjust accordingly!

   


    float shadowFactor = PCF(u_ShadowMap, PosLightMap);

    // shadowFactor typically is 1.0 if fully lit, 0.0 if in shadow,
    // or partial if the hardware does PCF.

    ////////////////////////////////////////////////////////////////////////////
    // 4) Combine lighting with the shadow factor
    ////////////////////////////////////////////////////////////////////////////
    // For example, multiply the diffuse portion by (PosLightMap)
    vec3 finalColor = shadowFactor * lighting;


    color = vec4(finalColor, 1.0);


    //color = vec4(shadowFactor, 0.0, 0.0, 1.0);


}