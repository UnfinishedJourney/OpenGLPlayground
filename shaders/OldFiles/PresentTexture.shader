#shader vertex
#version 460 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord;

out vec2 v_TexCoord;

void main()
{
    v_TexCoord = texCoord;
    gl_Position = vec4(position, 0.0, 1.0);
}

#shader fragment
#version 460 core

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;

uniform sampler2D u_SceneTexture;

void main()
{
    vec4 texColor = texture(u_SceneTexture, v_TexCoord);
    color = texColor;
}