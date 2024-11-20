#version 460 core
#include "Common/Common.shader"

in vec3 position;
in vec2 texCoord;

out vec2 v_TexCoord;

uniform mat4 u_Model;
uniform mat3 u_NormalMatrix;

void main()
{
    gl_Position = u_Proj * u_View * u_Model * vec4(position, 1.0);
    v_TexCoord = texCoord;
}