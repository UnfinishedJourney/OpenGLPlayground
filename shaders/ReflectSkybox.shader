#shader vertex
#version 460 core

#include "Common/Common.shader"
uniform mat4 u_Model;
uniform mat3 u_NormalMatrix;

layout (location = 0) in vec3 positions; 
layout (location = 1) in vec3 normals; 

out vec3 ReflectDir; // Output reflection direction

void main() {
    vec3 worldPos = vec3(u_Model * vec4(positions, 1.0));
    
    // Transform and normalize normal to world space
    vec3 worldNorm = normalize(mat3(u_Model) * normals);
    
    // Compute view direction (from fragment to camera)
    vec3 viewDir = normalize(u_CameraPos.xyz - worldPos);
    
    // Compute reflection direction
    ReflectDir = reflect(-viewDir, worldNorm);
    
    // Transform vertex position to clip space
    gl_Position = u_Proj * u_View * u_Model * vec4(positions, 1.0);
}

#shader fragment
#version 460 core

layout(location = 0) out vec4 color;
layout(binding = 0) uniform samplerCube CubeMapTex; // Cube map sampler
in vec3 ReflectDir; 

void main()
{
    vec3 reflectColor = texture(CubeMapTex, ReflectDir).rgb;
    color = vec4(reflectColor, 1.0);
}