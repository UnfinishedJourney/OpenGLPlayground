#shader vertex
#version 460 core
#include "Common/Common.shader"
#include "Common/Lights.shader"

layout(location = 0) in vec3 position;
out vec3 LightColor;

void main()
{
    uint lightIndex = gl_InstanceID;
    //if (lightIndex >= numLights) {
        // Optional: Handle out-of-bounds instances
        //gl_Position = vec4(0.0);
        //return;
    //}

    // Retrieve light position from SSBO
    vec3 lightPos = lightsData[lightIndex].position.xyz;

    // Transform the sphere's vertex position
    //gl_Position = u_MVP * vec4(position + vec3(lightPos.x, lightPos.y, lightPos.z), 1.0);
    gl_Position = u_Proj*u_View * vec4(position + lightPos, 1.0);
    LightColor = lightsData[lightIndex].color.xyz;
}

#shader fragment
#version 460 core

layout(location = 0) out vec4 color;

in vec3 LightColor;

void main()
{
    //uint lightIndex = gl_InstanceID;
    //if (lightIndex >= numLights) {
        //color = vec4(1.0); // Default white color
        //return;
    //}

    float normalizedDepth = gl_FragCoord.z / gl_FragCoord.w;
    
    // Assign depth to the blue channel for visualization
    color = vec4(LightColor, 1.0);
}