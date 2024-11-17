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

    vec3 lightPos = lightsData[lightIndex].position.xyz;

    gl_Position = u_Proj*u_View * vec4(position + lightPos, 1.0);
    LightColor = lightsData[lightIndex].color.xyz;
}