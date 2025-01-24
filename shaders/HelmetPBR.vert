#version 460 core

layout (location = 0) in vec3 positions; 
layout (location = 1) in vec3 normals; 
layout (location = 2) in vec3 tangents;
layout (location = 3) in vec2 uvs;

uniform mat4 u_Model;

out vec2 tc;           // Texture coordinates
out vec3 fragPos;      // Fragment position in world space
out vec3 normal;       // Normal in world space
out mat3 TBN;          // Tangent, Bitangent, Normal matrix

#include "Common/Common.shader"

void main()
{
    tc = uvs;

    vec4 worldPosition = u_Model * vec4(positions, 1.0);
    fragPos = worldPosition.xyz;

    // Transform normal to world space
    normal = normalize(mat3(transpose(inverse(u_Model))) * normals);

    // Compute Tangent, Bitangent, Normal (TBN) matrix
    vec3 T = normalize(mat3(u_Model) * tangents);
    vec3 N = normal;
    vec3 B = cross(N, T);
    TBN = mat3(T, B, N); 

    // Final vertex position in clip space
    gl_Position = u_Proj * u_View * worldPosition;
}