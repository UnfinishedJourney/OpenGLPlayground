#shader vertex
#version 460 core
#include "Common/Common.shader"

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;

out vec2 v_TexCoord;

uniform mat4 u_MVP;
out vec3 LightColor;

void main()
{
    gl_Position = u_MVP * vec4(position, 1.0);
    LightColor = 
    v_TexCoord = texCoord;
}

#shader fragment
#version 460 core

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;

uniform sampler2D u_Texture;

void main()
{
    vec4 texColor = texture(u_Texture, v_TexCoord);
    color = texColor;
    //color = vec4(v_TexCoord, 0.0, 1.0);
}