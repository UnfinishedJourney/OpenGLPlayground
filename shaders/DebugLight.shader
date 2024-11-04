#shader vertex
#version 460 core
#include "Common/Common.shader"

void main()
{
    gl_Position = u_View * u_Proj * lightsData[i].position.xyz;
}

#shader fragment
#version 460 core

layout(location = 0) out vec4 color;

void main()
{
    color.xyz = lightsData[i].color.xyz;
}