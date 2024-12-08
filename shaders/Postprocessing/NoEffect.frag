#version 460 core

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;

uniform sampler2D u_SceneTexture;

void main()
{
    vec4 texColor = texture(u_SceneTexture, v_TexCoord);
    color = texColor;
}