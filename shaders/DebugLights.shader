#shader vertex
#version 460 core
#include "Common/Common.shader"
#include "Common/Lights.shader"

layout(location = 0) in vec3 position;

uniform mat4 u_MVP;
out vec3 LightColor;

void main()
{
    uint lightIndex = gl_InstanceID;
    if (lightIndex >= numLights) {
        // Optional: Handle out-of-bounds instances
        gl_Position = vec4(0.0);
        return;
    }

    // Retrieve light position from SSBO
    vec3 lightPos = lightsData[lightIndex].position.xyz;
    LightColor = lightsData[lightIndex].color.xyz;

    // Transform the sphere's vertex position
    gl_Position = u_MVP * vec4(position + lightPos, 1.0);
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

    color = vec4(LightColor, 1.0);
}