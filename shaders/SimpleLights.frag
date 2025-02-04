#version 460 core

#include "Common/Common.shader"
#include "Common/LightsFunctions.shader"

layout(location = 0) out vec4 color;
in vec3 FragPos;
in vec3 Normal;

void main()
{
    color = vec4(vec3(0.25, 0.53, 0.63), 1.0);
    //return;
    // Calculate view direction
    vec3 viewDir = normalize(u_CameraPos.xyz - FragPos);
      
    // Normalize the normal vector
    vec3 norm = normalize(Normal);
    
    // Choose lighting model based on uniform
    vec3 lighting = CalculatePhongLighting(norm, viewDir, FragPos);

    lighting.x = max(lighting.x, 0.0);
    lighting.y = max(lighting.y, 0.0);
    lighting.z = max(lighting.z, 0.0);
    //lighting = CalculateToonLighting(norm, viewDir, FragPos);
    
    color = vec4(lighting + vec3(0.25, 0.53, 0.63), 1.0);
}