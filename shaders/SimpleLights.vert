#version 460 core

#include "Common/Common.shader"
uniform mat4 u_Model;
uniform mat3 u_NormalMatrix;

layout (location = 0) in vec3 positions; 
layout (location = 1) in vec3 normals; 

out vec3 FragPos;
out vec3 Normal;

void main() {
    vec4 worldPos = u_Model * vec4(positions, 1.0);
    FragPos = worldPos.xyz;
    
    // Transform normal to world space and normalize
    Normal = normalize(u_NormalMatrix * normals);
    
    // Final vertex position in clip space
    gl_Position = u_Proj * u_View * worldPos;
}
