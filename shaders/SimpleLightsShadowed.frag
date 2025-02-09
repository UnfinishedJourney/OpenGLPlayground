#version 460 core

#include "Common/Common.shader"
#include "Common/LightsFunctions.shader"

layout(location = 0) out vec4 color;

in vec4 PosLightMap;  // Shadow coords (after bias)
// Inputs from the vertex shader
in vec3 FragPos;
in vec3 Normal;

layout(binding = 10) uniform sampler2DShadow u_ShadowMap;

float PCF(sampler2DShadow shadowMap, vec4 shadowCoord, int kernelSize)
{
    // do the perspective divide
    vec3 sc = shadowCoord.xyz / shadowCoord.w;

    // if out of [0..1], no shadow (treat as lit)
    if(any(lessThan(sc, vec3(0.0))) || any(greaterThan(sc, vec3(1.0))))
        return 1.0;

    // 3x3 PCF
    
    float sum = 0.0;
    float texSize = float( textureSize(u_ShadowMap, 0 ).x ); 
    float offset = 1.0 / texSize;
    int range = kernelSize / 2;

    for(int yy=-range; yy<=range; yy++)
    {
        for(int xx=-range; xx<=range; xx++)
        {
            vec4 offCoord = shadowCoord;
            offCoord.xy += vec2(xx, yy) * offset;
            float bias = 0.0005;
            offCoord.z -= bias;
            sum += textureProj(shadowMap, offCoord);
        }
    }
    return sum / (kernelSize * kernelSize);
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

   


    float shadowFactor = PCF(u_ShadowMap, PosLightMap, 3);

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