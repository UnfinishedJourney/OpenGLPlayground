#version 460 core

in vec2 tc;           // Texture coordinates
in vec3 fragPos;      // Fragment position in world space
in vec3 normal;       // Normal in world space
in mat3 TBN;          // TBN matrix

out vec4 out_FragColor; // Output fragment color

// Textures
layout(binding = 0) uniform sampler2D texBRDF_LUT;
layout(binding = 1) uniform sampler2D texAlbedo;
layout(binding = 2) uniform sampler2D texNormal;
layout(binding = 3) uniform sampler2D texMetalRoughness;
layout(binding = 4) uniform sampler2D texAO;
layout(binding = 5) uniform sampler2D texEmissive;


void main()
{
    vec3 albedo = texture(texEmissive, tc).rgb;
    out_FragColor = vec4(albedo, 1.0); 
}