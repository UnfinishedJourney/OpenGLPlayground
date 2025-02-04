#version 460 core

layout(location = 0) in vec3 position;

uniform mat4 u_ShadowMatrix; // Bias * LightProj * LightView

void main()
{
    gl_Position = u_ShadowMatrix * vec4(position, 1.0);
}
