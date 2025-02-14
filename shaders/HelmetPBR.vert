#version 460 core

layout (location = 0) in vec3 positions; 
layout (location = 1) in vec3 normals; 
layout (location = 2) in vec3 tangents;
layout (location = 3) in vec2 uvs;


out vec2 uv;           // Texture coordinates
out vec3 fragPos;      // Fragment position in world space
out vec3 normal;       // Normal in world space
out mat3 TBN;          // Tangent, Bitangent, Normal matrix

#include "Common/Common.shader"

void main()
{
    uv = uvs;

    vec4 worldPosition = vec4(positions, 1.0);
    fragPos = positions;

    normal = normals;

    // Compute Tangent, Bitangent, Normal (TBN) matrix
    vec3 T = normalize(tangents);
    vec3 N = normal;
    vec3 B = cross(N, T);
    TBN = mat3(T, B, N); 

    // Final vertex position in clip space
    gl_Position = u_Proj * u_View * worldPosition;
}