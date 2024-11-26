#version 460 core

#include "Common/Common.shader"
uniform mat4 u_Model;

layout (location = 0) in vec3 positions; 


void main() {
    gl_Position = u_Proj * u_View * u_Model * vec4(positions, 1.0);
}