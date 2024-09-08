#shader vertex
#version 330 core

layout(location = 0) in vec3 positions;
layout(location = 1) in vec3 normals;
layout(location = 2) in vec2 uvs;

out vec3 v_Normals;
out vec2 v_TexCoords;

uniform mat4 u_MVP;

void main()
{
    gl_Position = u_MVP * vec4(positions, 1.0);
    v_Normals = normals.xyz;
    v_TexCoords = uvs;
}

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;
in vec3 v_Normals;
in vec2 v_TexCoords;

uniform sampler2D u_Texture;

void main()
{
    vec4 texColor = texture(u_Texture, v_TexCoords.xy);
    color = texColor;
}