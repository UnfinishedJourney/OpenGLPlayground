#version 460 core

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;

layout(binding = 0) uniform sampler2D u_SceneTexture;
layout(binding = 1) uniform sampler2D u_TextureToDraw;

void main()
{
    vec4 texColor = texture(u_TextureToDraw, v_TexCoord);
    color = texColor;
}