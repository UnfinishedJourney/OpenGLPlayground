#version 460 core

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;

layout(binding = 0) uniform sampler2D u_Texture;

void main()
{
    color = vec4(1.0, 0.0, 0.0, 1.0);
    //return;
    vec4 texColor = texture(u_Texture, v_TexCoord);
    color = texColor;
}