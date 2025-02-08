#version 460 core
#include "Common/Common.shader"

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normals; 
layout (location = 2) in vec2 texCoord;

out vec3 wPos;
out vec3 wNormal;
out vec2 uv;
out vec4 PosLightMap;

uniform mat4 u_ShadowMatrix;

void main()
{
    wPos = position;
    gl_Position = u_Proj * u_View * vec4(wPos, 1.0);
    PosLightMap = u_ShadowMatrix * vec4(position, 1.0);
    wNormal = normals;
    uv = texCoord;
}