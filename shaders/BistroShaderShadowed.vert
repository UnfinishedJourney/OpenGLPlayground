#version 460 core
#include "Common/Common.shader"

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal; 
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec2 texCoord;

out vec3 wPos;
out vec3 wNormal;
out vec2 uv;
out vec4 PosLightMap;
out mat3 TBN;          // Tangent, Bitangent, Normal matrix

uniform mat4 u_ShadowMatrix;

void main()
{
    wPos = position;
    gl_Position = u_Proj * u_View * vec4(wPos, 1.0);
    PosLightMap = u_ShadowMatrix * vec4(position, 1.0);
    wNormal = normal;
    uv = texCoord;

    vec3 T = normalize(tangent);
    vec3 N = normal;
    vec3 B = cross(N, T);
    TBN = mat3(T, B, N); 
}