#version 460 core

layout(location = 0) in vec3 position;

#include "../Common/Common.shader"

out vec3 Vec;

void main() {
    Vec = position;
    mat4 noTranslationView = mat4(mat3(u_View));
    gl_Position = u_Proj * noTranslationView * vec4(300.0*position , 1.0);
}
