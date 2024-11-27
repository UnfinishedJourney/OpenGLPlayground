#shader vertex
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoords;

out vec2 TexCoords;

void main()
{
    TexCoords = a_TexCoords;
    gl_Position = vec4(a_Position, 1.0);
}

#shader fragment
#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D u_BRDFLUT;

void main()
{
    vec2 brdf = texture(u_BRDFLUT, TexCoords).rg;
    FragColor = vec4(brdf, 0.0, 1.0);
}