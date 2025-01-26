#version 460 core

#include "Common/Common.shader"

layout (location = 0) in vec3 positions; 
layout (location = 1) in vec3 normals; 

out vec3 FragPos;
out vec3 Normal;

void main() {
    FragPos = positions;
    
    // Transform normal to world space and normalize
    Normal = normalize(normals);
    
    // Final vertex position in clip space
    gl_Position = u_Proj * u_View * vec4(positions, 1.0);
}
