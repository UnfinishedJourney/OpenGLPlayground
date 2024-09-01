#shader vertex
#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 normals;
layout(location = 2) in vec2 uvs;

out vec3 v_Normal;
out vec2 v_TexCoord;

uniform mat4 u_MVP;

void main()
{
    gl_Position = u_MVP * position;
    v_TexCoord = uvs;
}

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;
in vec3 v_Normal;
in vec2 v_TexCoord;

void main()
{
    color = vec4(v_TexCoord.x, v_TexCoord.y, v_Normal.x, 1.0);
}