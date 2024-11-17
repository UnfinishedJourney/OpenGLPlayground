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