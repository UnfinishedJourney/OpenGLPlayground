#version 460 core

#include "Common/Common.shader"
#include "Common/LightsFunctions.shader"
layout(location = 0) out vec4 color;
in vec3 FragPos;
in vec3 Normal;

void main()
{
    // Calculate view direction
    vec3 viewDir = normalize(u_CameraPos.xyz - FragPos);
      
    // Normalize the normal vector
    vec3 norm = normalize(Normal);
    
    // Choose lighting model based on uniform
    //vec3 lighting = CalculatePhongLighting(norm, viewDir, FragPos);

    vec3 lighting = CalculateToonLighting(norm, viewDir, FragPos);
    
    color = vec4(lighting, 1.0);
    color.r = 0.2;
}