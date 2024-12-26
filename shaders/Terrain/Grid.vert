#version 460 core

layout (location = 0) in vec3 aPos;

#include "../Common/Common.shader"

out vec2 uv;
out vec2 camPos;
uniform float gridSize = 100.0;

void main()
{
    vec3 vpos = vec3(aPos.x, 0.0, aPos.y)* gridSize;
    vpos.x += u_CameraPos.x;
    vpos.z += u_CameraPos.z;
    vec4 worldPos = vec4(vpos, 1.0);
    gl_Position = u_Proj * u_View * worldPos;
    camPos = u_CameraPos.xz;
    // Use world position's x and z for UV coordinates
    uv = vpos.xz;
}