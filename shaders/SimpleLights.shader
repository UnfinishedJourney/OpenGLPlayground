#shader vertex
#version 460 core

#include "Common/Common.shader"
uniform mat4 u_MVP;
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

#shader fragment
#version 460 core

#include "Common/Common.shader"
#include "Common/Lights.shader"
layout(location = 0) out vec4 color;
in vec3 FragPos;
in vec3 Normal;

void main()
{
    vec3 viewDir = normalize(u_CameraPos.xyz - FragPos);
      
    // Normalize the normal vector
    vec3 norm = normalize(Normal);
    
    // Calculate lighting using Phong model
    //vec3 lighting = CalculatePhongLighting(norm, viewDir, FragPos);
    
    // Set the fragment color
    color = vec4(0.0, 1.0, 0.0, 1.0);
    color.xyz = lightsData[0].color.xyz;
}
