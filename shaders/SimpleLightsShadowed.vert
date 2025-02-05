#version 460 core

#include "Common/Common.shader"
#include "Common/Lights.shader"

layout (location = 0) in vec3 positions; 
layout (location = 1) in vec3 normals; 

out vec4 PosLightMap;
out vec3 FragPos;
out vec3 Normal;

uniform mat4 u_ShadowMatrix;

void main() {
    FragPos = positions;
    
    // Transform normal to world space and normalize
    Normal = normalize(normals);
    

    PosLightMap = u_ShadowMatrix * vec4(positions, 1.0);

    // Final vertex position in clip space
    gl_Position = u_Proj * u_View * vec4(positions, 1.0);

}
